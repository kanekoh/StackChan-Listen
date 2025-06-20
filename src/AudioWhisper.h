#ifndef _AUDIOWHISPER_H
#define _AUDIOWHISPER_H

#include "AudioWhisper.h"

class AudioWhisper {
  byte* record_buffer;
 public:
  AudioWhisper();
  ~AudioWhisper();

  const byte* GetBuffer() const { return record_buffer; }
  size_t GetSize() const;

  void Record();
  static void CreateWavHeader(byte* header, int dataSize) ;
};

#endif // _AUDIOWHISPER_H