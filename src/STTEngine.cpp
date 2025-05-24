#include "STTEngine.h"
#include "AudioWhisper.h"
#include "Whisper.h"
#include "Audio.h"
#include "CloudSpeechClient.h"
#include "rootCAgoogle.h"
#include "rootCACertificate.h"  // OpenAI

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
