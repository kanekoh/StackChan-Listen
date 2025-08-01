#include <M5Unified.h>
#include "Audio.h"
#include "I2SBlockingGuard.h"

Audio::Audio() {
  wavData = (typeof(wavData))heap_caps_malloc(record_size * sizeof(int16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
//  wavData = (typeof(wavData))heap_caps_malloc(record_size * sizeof(int16_t),  MALLOC_CAP_8BIT);
  memset(wavData, 0 , record_size * sizeof(int16_t));
}

Audio::~Audio() {
  delete wavData;
}

void Audio::CreateWavHeader(byte* header, int waveDataSize){
  header[0] = 'R';
  header[1] = 'I';
  header[2] = 'F';
  header[3] = 'F';
  unsigned int fileSizeMinus8 = waveDataSize + 44 - 8;
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
  header[40] = (byte)(waveDataSize & 0xFF);
  header[41] = (byte)((waveDataSize >> 8) & 0xFF);
  header[42] = (byte)((waveDataSize >> 16) & 0xFF);
  header[43] = (byte)((waveDataSize >> 24) & 0xFF);
}

void Audio::Record() {
  CreateWavHeader(paddedHeader, wavDataSize);
  I2SBlockingGuard guard(I2SMode::Recording);
  M5.Speaker.end(); 
  delay(10);
  M5.Mic.begin();
  int rec_record_idx;
  for (rec_record_idx = 0; rec_record_idx < record_number; rec_record_idx++) {
    auto data = &wavData[rec_record_idx * record_length];
    M5.Mic.record(data, record_length, record_samplerate);
  }
  M5.Mic.end();
  delay(10);
  M5.Speaker.begin(); 
}