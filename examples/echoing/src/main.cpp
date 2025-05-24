#include <M5Unified.h>
#include <Avatar.h>
#include "WiFiHelper.h" 
#include "SpeechEngine.h"
#include "SDUtils.h"
#include "STTEngine.h"
#ifndef AUDIO_OUTPUT_M5_SPEAKER_H
#define AUDIO_OUTPUT_M5_SPEAKER_H
#include "AudioOutputM5Speaker.h"
#endif

using namespace m5avatar;

Avatar avatar;
AudioOutputM5Speaker* audioOut = nullptr;
STTEngine stt;
bool waitingForTouch = true;

void outputMessage(String message) {
  Serial.println(message);
  M5.Display.println(message);
}

void speechTask(void*) {
  for (;;) {
    SpeechEngine::processSpeechQueue();
    delay(100);
  }
}

void playbackTask(void*) {
  for (;;) {
    SpeechEngine::playback();
    delay(5);
  }
}

void lipSync(void *args)
{
  float gazeX, gazeY;
  int level = 0;
  DriveContext *ctx = (DriveContext *)args;
  Avatar *avatar = ctx->getAvatar();
  for (;;)
  {
    level = abs(*audioOut->getBuffer());
    if(level<100) level = 0;
    if(level > 15000)
    {
      level = 15000;
    }
    float open = (float)level/15000.0;
    avatar->setMouthOpenRatio(open);
    avatar->getGaze(&gazeY, &gazeX);
    avatar->setRotation(gazeX * 5);
    delay(50);
  }
}

void setup() {
  M5.begin();
  Serial.begin(115200);  // ★これを追加
  delay(100);            // ★シリアル同期用の短い待機
  std::vector<String> keys;

    // スピーカーの設定
  auto spk_cfg = M5.Speaker.config();
  spk_cfg.sample_rate = 96000;
  spk_cfg.task_pinned_core = APP_CPU_NUM;
  M5.Speaker.config(spk_cfg);
  M5.Speaker.begin();
  M5.Speaker.setVolume(100);
  audioOut = new AudioOutputM5Speaker(&M5.Speaker, 0);


  outputMessage("Initilize SD card");
  if (initSDCard()) {
    if (readLinesFromSD("/apikey.txt", keys) && keys.size() >= 3) {
      String openaiKey = keys[0];
      String voicevoxKey = keys[1];
      String sttKey = keys[2];
      SpeechEngine::initSpeechEngine(voicevoxKey, "3", audioOut);  // APIキーだけ渡す
      stt.begin(sttKey,  (openaiKey != sttKey) );  // Whisperを使う場合
      outputMessage("Scceeded to read /apikey.txt");
    } else {
      M5.Lcd.println("APIキー読み込み失敗");
    }
  } else {
    outputMessage("Initilize SD card failed.");
  }

  outputMessage("Trying to connect to Wi-Fi...");
  bool success = WiFiHelper::setupWiFi();  
  if (success) {
    String ip = WiFi.localIP().toString();
    outputMessage("Connected! IP: " + ip);
  } else {
    outputMessage("Wi-Fi connection failed. Check settings.");
  }


  delay(1000);
  avatar.init();  
  avatar.addTask(speechTask, "speech", 6144);
  avatar.addTask(playbackTask, "playback", 6144);
  avatar.addTask(lipSync, "lipSync");
  avatar.setSpeechFont(&fonts::efontJA_16);
  delay(1000);

  // 最初のあいさつ
  SpeechEngine::enqueueText("やっほー！スタックチャンだよ。今日はしゃべり方のテストをするね！");
}

unsigned long lastSpeak = 0;
unsigned long interval = 70000; // 約70秒間隔で発話
int mode = 0; // 0 = 旧方式、1 = キュー方式
bool pending = false;

void loop() {
  M5.update();


  if (waitingForTouch) {
    auto touchCount = M5.Touch.getCount();
    if (touchCount > 0) {
      auto t = M5.Touch.getDetail();
      if (t.wasPressed()) {
        // 顔全体をトリガーにしたいなら、全体を対象に
        if (true /* もしくは条件 t.x, t.y */) {
          Serial.println("顔タップ！録音開始");
          waitingForTouch = false;

          // 表情やLEDで「録音中」を表現
          // avatar.setExpression(Expression::Happy); など

          avatar.setExpression(Expression::Happy);
          avatar.setSpeechText("きいてるよ");
          delay(60);
          String text = stt.transcribe();
          avatar.setSpeechText("");
          avatar.setExpression(Expression::Neutral);

          // 録音終了後、表示クリア
          SpeechEngine::enqueueText(text);
          waitingForTouch = true;
        }
      }
    }
  }

  delay(10);
}