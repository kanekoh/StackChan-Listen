#include <ArduinoJson.h>
#include "Whisper.h"

namespace {
constexpr char* API_HOST = "api.openai.com";
constexpr int API_PORT = 443;
constexpr char* API_PATH = "/v1/audio/transcriptions";
}  // namespace

Whisper::Whisper(const char* root_ca, const char* api_key)
  : client(), key(api_key), rootCA(root_ca) {
  client.setCACert(root_ca);
  client.setTimeout(10000); 
}

Whisper::~Whisper() {
  client.stop();
}

String Whisper::Transcribe(AudioWhisper* audio) {
  String boundary = generateBoundary();
  return sendMultipartRequest(audio->GetBuffer(), audio->GetSize(), boundary);
}

String Whisper::TranscribeFromBuffer(const byte* buffer, size_t length) {
  String boundary = generateBoundary();
  return sendMultipartRequest(buffer, length, boundary);
}

bool Whisper::IdentifyFromBuffer(const byte* buffer, size_t len, String& speaker, String& kana, float& score) {
  const char* host = "192.168.10.22";  // IPアドレスかmDNS名に置換
  const int port = 8082;
  const String boundary = "----ESP32Boundary";

  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("❌ Connection failed.");
    return false;
  }

  // ---- ヘッダ + バウンダリ ----
  String head =
    "--" + boundary + "\r\n"
    "Content-Disposition: form-data; name=\"audio\"; filename=\"temp.wav\"\r\n"
    "Content-Type: audio/wav\r\n\r\n";

  String tail = "\r\n--" + boundary + "--\r\n";
  int contentLength = head.length() + len + tail.length();

  // ---- HTTPヘッダ ----
  client.printf("POST /identify HTTP/1.1\r\n");
  client.printf("Host: %s\r\n", host);
  client.println("Connection: close");
  client.printf("Content-Type: multipart/form-data; boundary=%s\r\n", boundary.c_str());
  client.printf("Content-Length: %d\r\n", contentLength);
  client.println();
  client.print(head);

  // ---- 音声データ送信 ----
  size_t sent = 0;
  while (sent < len) {
    size_t chunk = std::min((size_t)512, len - sent);
    client.write(buffer + sent, chunk);
    sent += chunk;
    delay(1); // 小休止
  }

  client.print(tail);
  client.flush();

  // ---- レスポンス受信 ----
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println("⏰ Timeout waiting for response.");
      client.stop();
      return false;
    }
  }

  // ---- ボディ抽出 ----
  String body = "";
  bool isBody = false;
  while (client.available()) {
    String line = client.readStringUntil('\n');
    if (isBody) {
      body += line;
    }
    if (line == "\r") {
      isBody = true;
    }
  }

  client.stop();

  // ---- JSONパース ----
  StaticJsonDocument<512> doc;
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    Serial.println("❌ Failed to parse JSON.");
    return false;
  }

  speaker = doc["name"] | "";
  kana    = doc["kana"] | speaker;
  score   = doc["score"] | 0.0;

  return true;
}


String Whisper::generateBoundary() {
  char boundary[64] = "------------------------";
  for (auto i = 0; i < 2; ++i) {
    ltoa(random(0x7fffffff), boundary + strlen(boundary), 16);
  }
  return String(boundary);
}

String Whisper::sendMultipartRequest(const byte* data, size_t length, const String& boundary) {
  // multipart ヘッダとフッタを生成
  const String header =
    "--" + boundary + "\r\n"
    "Content-Disposition: form-data; name=\"model\"\r\n\r\nwhisper-1\r\n"
    "--" + boundary + "\r\n"
    "Content-Disposition: form-data; name=\"language\"\r\n\r\nja\r\n"
    "--" + boundary + "\r\n"
    "Content-Disposition: form-data; name=\"file\"; filename=\"speak.wav\"\r\n"
    "Content-Type: application/octet-stream\r\n\r\n";

  const String footer = "\r\n--" + boundary + "--\r\n";

  // HTTP リクエスト送信
  client.printf("POST %s HTTP/1.1\n", API_PATH);
  client.printf("Host: %s\n", API_HOST);
  client.println("Accept: */*");
  client.printf("Authorization: Bearer %s\n", key.c_str());
  client.printf("Content-Length: %d\n", header.length() + length + footer.length());
  client.printf("Content-Type: multipart/form-data; boundary=%s\n", boundary.c_str());
  client.println();

  client.print(header);
  client.flush();

  size_t remaining = length;
  const byte* ptr = data;
  while (remaining > 0) {
    size_t chunk = (remaining > 512) ? 512 : remaining;
    client.write(ptr, chunk);
    client.flush();
    ptr += chunk;
    remaining -= chunk;
  }

  client.print(footer);
  client.flush();

  // レスポンス待機
  const auto start = ::millis();
  while (client.available() == 0) {
    if (::millis() - start > 10000) {
      Serial.println(">>> Client Timeout !");
      return "";
    }
  }

  // レスポンスパース
  bool isBody = false;
  String body;
  while (client.available()) {
    String line = client.readStringUntil('\r');
    if (isBody) {
      body += line;
    } else if (line == "\n") {
      isBody = true;
    }
  }

  StaticJsonDocument<200> doc;
  deserializeJson(doc, body);
  return doc["text"].as<String>();
}
