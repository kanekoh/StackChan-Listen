#include "ListenEngine.h"
#include <cstring>  // for memcpy
#include <M5Unified.h>
#include <math.h>

bool ListenEngine::begin(int sample_rate) {
  // config_.sample_rate = sample_rate;
  micInput_ = new MicInput();
  micInput_->begin(sample_rate);

  // if (!vad_.init(config_)) {
  //   Serial.println("VAD initialization failed.");
  //   return false;
  // }

  // frameLen_ = config_.frame_length();
  // bufferLen_ = config_.sample_rate * maxSeconds_;  // 5秒分
  ringBuffer_ = new int16_t[bufferLen_];
  ringHead_ = 0;
  ringCount_ = 0;

  return true;
}

void ListenEngine::end() {
  micInput_->end();
  // vad_.deinit();
  delete[] ringBuffer_;
  delete micInput_;
}

bool ListenEngine::listen(std::vector<int16_t>& out_wav_data) {
  int16_t* frame = new int16_t[frameLen_];

  while (true) {
    if (!micInput_->readFrame(frame, frameLen_)) {
      Serial.println("Failed to read mic frame.");
      break;
    }

    // リングバッファにデータを保存
    for (int i = 0; i < frameLen_; ++i) {
      ringBuffer_[ringHead_] = frame[i];
      ringHead_ = (ringHead_ + 1) % bufferLen_;
    }
    ringCount_ = std::min(ringCount_ + frameLen_, bufferLen_);

    // // VAD処理
    // auto state = vad_.process(frame);
    // if (state == simplevox::VadState::Detected) {
    //   // Detected: 検出完了 → 5秒分のデータを出力
    //   out_wav_data.clear();
    //   out_wav_data.reserve(ringCount_);

    //   size_t start = (ringHead_ + bufferLen_ - ringCount_) % bufferLen_;
    //   for (size_t i = 0; i < ringCount_; ++i) {
    //     size_t idx = (start + i) % bufferLen_;
    //     out_wav_data.push_back(ringBuffer_[idx]);
    //   }

    //   delete[] frame;
    //   return true;
    // }
  }

  delete[] frame;
  return false;
}
