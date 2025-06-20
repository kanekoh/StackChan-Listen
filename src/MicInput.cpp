// MicInput.cpp
#include "MicInput.h"
#include <M5Unified.h>

void MicInput::begin(int sampleRate) {
    sample_rate_ = sampleRate;
    M5.Mic.begin();
}

bool MicInput::readFrame(int16_t* buffer, size_t length) {
    return M5.Mic.record(buffer, length, sample_rate_) > 0;
}

void MicInput::end() {
    M5.Mic.end();
}
