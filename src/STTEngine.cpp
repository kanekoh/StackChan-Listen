#include "STTEngine.h"
#include "AudioWhisper.h"
#include "Whisper.h"
#include "Audio.h"
#include "CloudSpeechClient.h"
#include "rootCAgoogle.h"
#include "rootCACertificate.h"  // OpenAI
#include <vector>

void STTEngine::begin(const String& sttKey, bool useGoogle) {
  sttKey_ = sttKey;
  useGoogle_ = useGoogle;
}

String STTEngine::transcribe() {
  String result = "";

  if (useGoogle_) {
    Audio* audio = new Audio();
    audio->Record();
    Serial.println("Record complete. Sending to Google STT...");
    CloudSpeechClient* client = new CloudSpeechClient(root_ca_google, sttKey_.c_str());
    result = client->Transcribe(audio);
    delete client;
    delete audio;
  } else {
    AudioWhisper* audio = new AudioWhisper();
    audio->Record();
    Serial.println("Record complete. Sending to OpenAI Whisper...");
    Whisper* client = new Whisper(root_ca_openai, sttKey_.c_str());
    result = client->Transcribe(audio);
    delete client;
    delete audio;
  }

  return result;
}

void STTEngine::setListenEngine(ListenEngine* engine) {
  listenEngine_ = engine;
}

// String STTEngine::transcribeWithVAD() {
//   if (!listenEngine_) {
//     Serial.println("ListenEngine not set.");
//     return "";
//   }

//   std::vector<int16_t> wav_data;
//   bool detected = listenEngine_->listen(wav_data);
//   if (!detected) {
//     Serial.println("No voice detected.");
//     return "";
//   }

//   Serial.println("Detected voice. Sending to Whisper...");

//   // WAVヘッダー付きバッファを作成
//   const int headerSize = 44;
//   size_t dataSize = wav_data.size() * sizeof(int16_t);
//   byte* buffer = new byte[dataSize + headerSize];
//   memset(buffer, 0, dataSize + headerSize);

//   // ヘッダー作成
//   AudioWhisper::CreateWavHeader(buffer, dataSize);

//   // 音声データをコピー
//   memcpy(buffer + headerSize, wav_data.data(), dataSize);

//   // Whisperへ送信
//   Whisper* client = new Whisper(root_ca_openai, sttKey_.c_str());
//   String result = client->TranscribeFromBuffer(buffer, dataSize + headerSize);
//   delete client;
//   delete[] buffer;
//   return result;
// }

STTResult STTEngine::transcribeWithSpeaker() {
  STTResult result;
  result.text = "";
  result.speaker = "";
  result.kana = "";
  result.score = 0;

  if (!listenEngine_) {
    Serial.println("ListenEngine not set.");
    return result;
  }

  std::vector<int16_t> wav_data;
  bool detected = listenEngine_->listen(wav_data);
  if (!detected) {
    Serial.println("No voice detected.");
    return result;
  }

  Serial.println("Detected voice. Sending to Whisper and Identify...");

  const int headerSize = 44;
  size_t dataSize = wav_data.size() * sizeof(int16_t);
  byte* buffer = new byte[dataSize + headerSize];
  memset(buffer, 0, dataSize + headerSize);

  AudioWhisper::CreateWavHeader(buffer, dataSize);
  memcpy(buffer + headerSize, wav_data.data(), dataSize);

  // Whisper
  Whisper* whisperClient = new Whisper(root_ca_openai, sttKey_.c_str());
  result.text = whisperClient->TranscribeFromBuffer(buffer, dataSize + headerSize);
  delete whisperClient;

  // Identify
  Whisper* identifyClient = new Whisper(root_ca_openai, sttKey_.c_str());  // 再利用でもOK
  if (!identifyClient->IdentifyFromBuffer(buffer, dataSize + headerSize, result.speaker, result.kana, result.score)) {
    Serial.println("❌ Identify failed.");
  } else {
    Serial.printf("🧑 話者: %s (%s), score=%.2f\n", result.speaker.c_str(), result.kana.c_str(), result.score);
  }
  delete identifyClient;

  delete[] buffer;
  return result;
}
