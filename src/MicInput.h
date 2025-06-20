// MicInput.h
#pragma once
#include <stdint.h>  // for int16_t
#include <stddef.h>  // for size_t

class MicInput {
public:
    void begin(int sampleRate = 16000);
    bool readFrame(int16_t* buffer, size_t length);
    void end();
    virtual ~MicInput() = default;

private:
    int sample_rate_ = 16000;
};
