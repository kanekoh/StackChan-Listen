#pragma once
#include <Arduino.h>

class STTEngine {
public:
  void begin(const String& sttKey, bool useGoogle = false);
  String transcribe();

private:
  String sttKey_;
  bool useGoogle_;
};
