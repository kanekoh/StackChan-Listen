// ListenEngine.h
#pragma once

#include <stdint.h>
#include <stddef.h>
#include "simplevox.h"
#include "MicInput.h"
#include <vector>
#include <Arduino.h>

class ListenEngine {
public:
    ListenEngine();
    ~ListenEngine();

    bool begin(int sample_rate = 16000);
    bool listen(std::vector<int16_t>& out_wav_data);
    void end();

private:
    simplevox::VadEngine vad_;
    simplevox::VadConfig config_;
    MicInput* micInput_ = nullptr;
    size_t maxSeconds_ = 5;  // 最大録音時間（秒）
    static const size_t ringBufferSize = 16000 * 5; // 5秒分 @ 16kHz
    int16_t* ringBuffer_ = nullptr;
    size_t ringWriteIndex_ = 0;
    size_t frameLen_ = 0;  // 1フレームあたりのサンプル数（例：160）
    size_t bufferLen_ = 0; // リングバッファの長さ
    size_t ringHead_ = 0; // リングバッファの先頭
    size_t ringCount_ = 0; // リングバッファに保存されているサンプル数
    byte* wavBuffer_ = nullptr;
    size_t wavLength_ = 0;


    void addToRingBuffer(const int16_t* data, size_t length);
    void copyFromRingBuffer(int16_t* dest, size_t length);
};
