# ESP32 Tamagotchi Emulator

STM32版TamagotchiエミュレータのESP32移植版です。

## サポートハードウェア

- ESP32 (基本機能)
- ESP32-S2/S3 (USB MSC機能付き)
- SSD1306 OLED ディスプレイ (128x64)
- 3ボタン入力
- RGB LED
- スピーカー/ブザー
- バッテリー電圧測定

## ピン配置 (デフォルト)

| 機能 | ESP32ピン |
|------|-----------|
| SSD1306 SCLK | GPIO18 |
| SSD1306 MOSI | GPIO23 |
| SSD1306 DC | GPIO2 |
| SSD1306 RST | GPIO4 |
| バックライト | GPIO5 |
| 左ボタン | GPIO12 |
| 中央ボタン | GPIO13 |
| 右ボタン | GPIO14 |
| 赤LED | GPIO25 |
| 緑LED | GPIO26 |
| 青LED | GPIO27 |
| スピーカー | GPIO32 |
| バッテリー測定 | GPIO36 |

## ビルド方法

### 前提条件

- ESP-IDF v4.4以上
- CMake 3.16以上

### ビルド手順

1. ESP-IDFセットアップ
```bash
. $HOME/esp/esp-idf/export.sh
```

2. プロジェクトビルド
```bash
cd espgotchi
idf.py build
```

3. フラッシュ書き込み
```bash
idf.py -p /dev/ttyUSB0 flash
```

4. モニタリング
```bash
idf.py -p /dev/ttyUSB0 monitor
```

## 設定

### WiFi設定 (将来拡張用)
```bash
idf.py menuconfig
```

### カスタムピン配置
`src/mcu/esp32/board_esp32.h`を編集してピン配置を変更できます。

## 機能

### 実装済み
- ✅ GPIO/入力処理
- ✅ SPI通信 (ディスプレイ)
- ✅ 時間管理/タイマー
- ✅ 音声出力 (PWM)
- ✅ バッテリー測定 (ADC)
- ✅ 電力管理
- ✅ SSD1306ディスプレイドライバ
- ✅ SPIFFSファイルシステム
- ✅ ROM管理機能
- ✅ NVS設定保存
- ✅ USB MSC (ESP32-S2/S3のみ)

### TamaLIB統合
オリジナルのTamagotchi P1エミュレーションエンジンを完全サポート：
- CPU エミュレーション
- LCD マトリックス制御
- 音声出力
- ボタン入力
- セーブステート機能

## ファイル構造

```
espgotchi/
├── CMakeLists.txt              # メインCMake設定
├── sdkconfig.defaults          # ESP32デフォルト設定
├── partitions.csv              # パーティション定義
├── main/
│   ├── CMakeLists.txt          # メインコンポーネント
│   └── esp32_main.c            # ESP32エントリーポイント
├── components/
│   └── tamalib/                # TamaLIBコンポーネント
├── src/
│   ├── main.c                  # アプリケーションメイン
│   ├── config.c/h              # 設定管理
│   ├── gfx.c/h                 # グラフィックス
│   ├── menu.c/h                # メニューシステム
│   ├── rom.c/h                 # ROM管理
│   ├── state.c/h               # ステート管理
│   ├── ssd1306.c/h             # ディスプレイドライバ
│   └── mcu/
│       ├── inc/                # HAL インターフェース
│       └── esp32/              # ESP32固有実装
└── libs/
    └── TamaLIB/                # Tamagotchiエミュレーションエンジン
```

## 使用方法

1. ROMファイル (`rom0.bin`) をSPIFFSパーティションに配置
2. ボタン操作でTamagotchiを操作
3. 設定メニューから各種設定を調整
4. USB MSC経由でファイル転送 (ESP32-S2/S3)

## トラブルシューティング

### よくある問題

**ディスプレイが表示されない**
- SPI配線を確認
- 電源電圧を確認 (3.3V)
- ピン設定を確認

**ボタンが反応しない**
- プルアップ抵抗を確認
- ピン設定を確認
- デバウンス設定を調整

**音が出ない**
- PWM設定を確認
- スピーカー/ブザー配線を確認
- 音量設定を確認

### ログレベル調整
```bash
idf.py menuconfig
# Component config -> Log output -> Default log verbosity
```

## ライセンス

GNU General Public License v2.0

## 貢献

プルリクエストやイシュー報告を歓迎します。

## 謝辞

- オリジナルMCUGotchiプロジェクト
- TamaLIB エミュレーションエンジン
- ESP-IDF フレームワーク