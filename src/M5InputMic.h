#pragma once

#include "MicInput.h"
#include <M5Unified.h>

class M5MicInput : public MicInput {
public:
  void begin() override {
    M5.Mic.begin();
  }

  void end() override {
    M5.Mic.end();
  }

  void record(int16_t* buffer, size_t length, uint32_t sampleRate) override {
    M5.Mic.record(buffer, length, sampleRate);
  }
};
