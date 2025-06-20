#pragma once
#include <Arduino.h>
#include "ListenEngine.h"

struct STTResult {
  String text;      // 認識された音声テキスト
  String speaker;   // 話者のID
  String kana;      // 話者のかな表記
  float score;      // 類似度スコア
};


class STTEngine {
  ListenEngine* listenEngine_ = nullptr; // 追加

public:
  void begin(const String& sttKey, bool useGoogle = false);
  void setListenEngine(ListenEngine* engine); // 追加
  String transcribe();
  String transcribeFromBuffer(const int16_t* buffer, size_t length);
  String transcribeWithVAD();  // 追加: VAD付きSTT
  STTResult transcribeWithSpeaker();

private:
  String sttKey_;
  bool useGoogle_;
};
