// MicInput.cpp
#include "MicInput.h"
#include <M5Unified.h>
#include "I2SBlockingGuard.h"

static I2SBlockingGuard* micGuard = nullptr;

void MicInput::begin(int sampleRate) {
    micGuard = new I2SBlockingGuard(I2SMode::Recording);
    sample_rate_ = sampleRate;
    M5.Speaker.end(); 
    delay(10);
    M5.Mic.begin();
}

bool MicInput::readFrame(int16_t* buffer, size_t length) {
    return M5.Mic.record(buffer, length, sample_rate_) > 0;
}

void MicInput::end() {
    M5.Mic.end();
    delay(10);
    M5.Speaker.end(); 
    delete micGuard;  
    micGuard = nullptr;
}
