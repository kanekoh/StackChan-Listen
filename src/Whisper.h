#ifndef _Whisper_H
#define _Whisper_H
#include <WiFiClientSecure.h>
#include "AudioWhisper.h"

class Whisper {
  WiFiClientSecure client;
  String key;
  const char* rootCA;  // ★ 追加：証明書保持用

public:
  Whisper(const char* root_ca, const char* api_key);
  ~Whisper();
  String Transcribe(AudioWhisper* audio);
  String TranscribeFromBuffer(const byte* buffer, size_t length);
  bool IdentifyFromBuffer(const byte* buffer, size_t len, String& speaker, String& kana, float& score);

private:
  String generateBoundary();
  String sendMultipartRequest(const byte* data, size_t length, const String& boundary);
};

#endif // _Whisper_H
