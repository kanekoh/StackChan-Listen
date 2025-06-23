#pragma once

#include "MicInput.h"
#include <M5Unified.h>
#include "I2SBlockingGuard.h"

class M5MicInput : public MicInput {
public:
  void begin() override {
    guard_ = new I2SBlockingGuard(I2SMode::Recording);  // 🔐 排他ロック取得
    M5.Speaker.end(); 
    delay(10);
    M5.Mic.begin();
  }

  void end() override {
    M5.Mic.end();
    delay(10);
    M5.Speaker.begin(); 
    delete guard_;  // 🔓 ロック解放
    guard_ = nullptr;
  }

  void record(int16_t* buffer, size_t length, uint32_t sampleRate) override {
    M5.Mic.record(buffer, length, sampleRate);
  }

private:
  I2SBlockingGuard* guard_ = nullptr;
};

