#include <M5Unified.h>
#include "AudioWhisper.h"
#include "I2SBlockingGuard.h"

//constexpr size_t record_number = 300/2;
constexpr size_t record_number = 400;
//constexpr size_t record_number = 200;
constexpr size_t record_length = 150;
constexpr size_t record_size = record_number * record_length;
constexpr size_t record_samplerate = 16000;
constexpr int headerSize = 44;

AudioWhisper::AudioWhisper() {
  const auto size = record_size * sizeof(int16_t) + headerSize + 8;
  // record_buffer = static_cast<byte*>(::heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  record_buffer = (byte*)malloc(size);
  ::memset(record_buffer, 0, size);
  Serial.printf("[AudioWhisper] malloc %p (size=%d)\n", record_buffer, size);
}

AudioWhisper::~AudioWhisper() {
  Serial.printf("[AudioWhisper] free %p\n", record_buffer);
  if (record_buffer) {
    // heap_caps_free(record_buffer);
    free(record_buffer);
    record_buffer = nullptr;
  }
}

size_t AudioWhisper::GetSize() const {
  return record_size * sizeof(int16_t) + headerSize;
}

int16_t* MakeHeader(byte* header) {
  const auto wavDataSize = record_number * record_length * 2;
  header[0] = 'R';
  header[1] = 'I';
  header[2] = 'F';
  header[3] = 'F';
  unsigned int fileSizeMinus8 = wavDataSize + headerSize - 8;
  header[4] = (byte)(fileSizeMinus8 & 0xFF);
  header[5] = (byte)((fileSizeMinus8 >> 8) & 0xFF);
  header[6] = (byte)((fileSizeMinus8 >> 16) & 0xFF);
  header[7] = (byte)((fileSizeMinus8 >> 24) & 0xFF);
  header[8] = 'W';
  header[9] = 'A';
  header[10] = 'V';
  header[11] = 'E';
  header[12] = 'f';
  header[13] = 'm';
  header[14] = 't';
  header[15] = ' ';
  header[16] = 0x10;  // linear PCM
  header[17] = 0x00;
  header[18] = 0x00;
  header[19] = 0x00;
  header[20] = 0x01;  // linear PCM
  header[21] = 0x00;
  header[22] = 0x01;  // monoral
  header[23] = 0x00;
  header[24] = 0x80;  // sampling rate 16000
  header[25] = 0x3E;
  header[26] = 0x00;
  header[27] = 0x00;
  header[28] = 0x00;  // Byte/sec = 16000x2x1 = 32000
  header[29] = 0x7D;
  header[30] = 0x00;
  header[31] = 0x00;
  header[32] = 0x02;  // 16bit monoral
  header[33] = 0x00;
  header[34] = 0x10;  // 16bit
  header[35] = 0x00;
  header[36] = 'd';
  header[37] = 'a';
  header[38] = 't';
  header[39] = 'a';
  header[40] = (byte)(wavDataSize & 0xFF);
  header[41] = (byte)((wavDataSize >> 8) & 0xFF);
  header[42] = (byte)((wavDataSize >> 16) & 0xFF);
  header[43] = (byte)((wavDataSize >> 24) & 0xFF);
  return (int16_t*)&header[headerSize];
}

void AudioWhisper::CreateWavHeader(byte* header, int dataSize) {
  const int headerSize = 44;
  unsigned int fileSizeMinus8 = dataSize + headerSize - 8;
  header[0] = 'R'; header[1] = 'I'; header[2] = 'F'; header[3] = 'F';
  header[4] = (byte)(fileSizeMinus8 & 0xFF);
  header[5] = (byte)((fileSizeMinus8 >> 8) & 0xFF);
  header[6] = (byte)((fileSizeMinus8 >> 16) & 0xFF);
  header[7] = (byte)((fileSizeMinus8 >> 24) & 0xFF);
  header[8] = 'W'; header[9] = 'A'; header[10] = 'V'; header[11] = 'E';
  header[12] = 'f'; header[13] = 'm'; header[14] = 't'; header[15] = ' ';
  header[16] = 0x10; header[17] = 0x00; header[18] = 0x00; header[19] = 0x00;
  header[20] = 0x01; header[21] = 0x00; header[22] = 0x01; header[23] = 0x00;
  header[24] = 0x80; header[25] = 0x3E; header[26] = 0x00; header[27] = 0x00;
  header[28] = 0x00; header[29] = 0x7D; header[30] = 0x00; header[31] = 0x00;
  header[32] = 0x02; header[33] = 0x00; header[34] = 0x10; header[35] = 0x00;
  header[36] = 'd'; header[37] = 'a'; header[38] = 't'; header[39] = 'a';
  header[40] = (byte)(dataSize & 0xFF);
  header[41] = (byte)((dataSize >> 8) & 0xFF);
  header[42] = (byte)((dataSize >> 16) & 0xFF);
  header[43] = (byte)((dataSize >> 24) & 0xFF);
}


void AudioWhisper::Record() {
  I2SBlockingGuard guard(I2SMode::Recording);
  M5.Speaker.end(); 
  Serial.println("[I2S] Speaker.end()");
  delay(10);
  M5.Mic.begin();
  Serial.println("[I2S] Mic.begin()");
  auto *wavData = MakeHeader(record_buffer);
  for (int rec_record_idx = 0; rec_record_idx < record_number; ++rec_record_idx) {
    auto data = &wavData[rec_record_idx * record_length];
    M5.Mic.record(data, record_length, record_samplerate);
  }
  M5.Mic.end();
  Serial.println("[I2S] Mic.end()");
  delay(10);
  M5.Speaker.begin(); 
  Serial.println("[I2S] Speaker.begin()");
}

void AudioWhisper::Record(std::vector<int16_t>& wav_data) {
  I2SBlockingGuard guard(I2SMode::Recording);
  M5.Speaker.end(); 
  Serial.println("[I2S] Speaker.end()");
  delay(10);
  M5.Mic.begin();
  Serial.println("[I2S] Mic.begin()");

  constexpr int sampleRate = 16000;
  constexpr int durationMs = 5000;
  constexpr int totalSamples = (sampleRate * durationMs) / 1000;
  constexpr int chunkSize = 160; // 10ms相当（任意）

  wav_data.clear();
  wav_data.reserve(totalSamples);

  std::vector<int16_t> chunk(chunkSize);

  for (int i = 0; i < totalSamples; i += chunkSize) {
    M5.Mic.record(chunk.data(), chunkSize, sampleRate);
    wav_data.insert(wav_data.end(), chunk.begin(), chunk.end());
  }

  M5.Mic.end();
  Serial.println("[I2S] Mic.end()");
  delay(10);
  M5.Speaker.begin(); 
  Serial.println("[I2S] Speaker.begin()");
}
