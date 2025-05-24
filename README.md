## 🧠 STTEngine ライブラリについて
概要
STTEngine は、M5Stack CoreS3 上で動作する Stack-chan 向けに開発された、クラウドベースの音声認識（Speech-To-Text）を行うライブラリです。
現在は以下の 2つの音声認識サービスに対応しています：

OpenAI Whisper API

Google Cloud Speech-to-Text API

このライブラリは、音声録音 → クラウドAPIへ送信 → テキストとして取得 という一連の処理をカプセル化しています。

## 特徴
AudioWhisper や Audio に依存した録音処理を内部でラップ

使用する STT サービス（Whisper または Google）を切り替え可能

録音中に Stack-chan の表情（例：Happy）を表示して、ユーザーにフィードバック

音声認識処理中の例外処理や非同期処理にも対応しやすい構造

ESP32上で動作し、録音音声は SDカード や SPIFFS に一時保存

## 使い方（基本）
ヘッダのインクルードと初期化
```
#include "STTEngine.h"

STTEngine stt;

void setup() {
  stt.begin("your_openai_or_google_api_key", false);  // false = Whisper を使う
}
```
音声からテキストを取得

```
String text = stt.transcribe();
Serial.println("認識結果: " + text);
```

表情の連携（オプション）
録音中に Stack-chan の顔を変化させることで、ユーザーにわかりやすくフィードバックできます。

```
avatar.setExpression(Expression::Happy);
avatar.setSpeechText("きいてるよ");

// 録音処理中...

avatar.setSpeechText("");
avatar.setExpression(Expression::Neutral);
```
※ avatar の連携は将来的に STTEngine に注入可能な設計も検討できます。

録音時間の制御
デフォルトは3秒です。変更するには transcribe() を次のように拡張することで対応可能です：

```
stt.transcribe(5000);  // 5秒間録音
```
（今後のアップデートで duration_ms を引数で受けるよう拡張予定）

## 依存ライブラリ
- M5Unified
- ArduinoJson

## 謝辞 🙏
このライブラリおよびその設計方針は、robo8080さんが開発された AI Stack-chan プロジェクトをベースに構築されています。
とくに音声認識やチャット処理の実装構造、UIインタラクションの設計から多くの学びと参考を得ました。
robo8080さんの公開と貢献に心より感謝いたします。

GitHub: https://github.com/robo8080/AI-StackChan