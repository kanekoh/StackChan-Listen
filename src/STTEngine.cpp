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

STTResult STTEngine::transcribeWithSpeaker() {
  STTResult result;
  result.text = "";
  result.speaker = "";
  result.kana = "";
  result.score = 0;

  // AudioWhisper で録音
  AudioWhisper* audio = new AudioWhisper();
  audio->Record();
  Serial.println("Record complete. Sending to OpenAI Whisper...");

  // Whisper で音声認識
  Whisper* whisperClient = new Whisper(root_ca_openai, sttKey_.c_str());
  result.text = whisperClient->TranscribeFromBuffer(audio->GetBuffer(), audio->GetSize());
  delete whisperClient;

  // Whisper で話者識別
  Whisper* identifyClient = new Whisper(root_ca_openai, sttKey_.c_str());
  if (!identifyClient->IdentifyFromBuffer(audio->GetBuffer(), audio->GetSize(), result.speaker, result.kana, result.score)) {
    Serial.println("❌ Identify failed.");
  } else {
    if (result.speaker.isEmpty()) result.speaker = "unknown";
    if (result.kana.isEmpty()) result.kana = "ななしのごんべ";
    Serial.printf("🧑 話者: %s (%s), score=%.2f\n", result.speaker.c_str(), result.kana.c_str(), result.score);
  }
  delete identifyClient;

  delete audio;
  return result;
}
