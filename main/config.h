#pragma once

#include "sdkconfig.h"

// =============================================================================
// 项目配置（只改本文件即可）
// =============================================================================

// --- 眼睛外观：只保留一个 #include ---
#define SYMMETRICAL_EYELID

//#include "data/defaultEye.h"    // 标准人眼（淡褐色）
//#include "data/dragonEye.h"      // 竖瞳龙/恶魔眼
// #include "data/noScleraEye.h"     // 大虹膜、无巩膜
//#include "data/goatEye.h"        // 横瞳山羊/Krampus 眼
// #include "data/newtEye.h"        // 蝾螈眼
// #include "data/terminatorEye.h"  // 终结者红眼
#include "data/catEye.h"         // 卡通猫眼（平面色）
// #include "data/owlEye.h"         // 猫头鹰 Minerva（建议关 TRACKING，头文件内 EYE_NO_TRACKING）
// #include "data/naugaEye.h"       // Nauga 眼球（建议关 TRACKING）
// #include "data/doeEye.h"         // 卡通鹿眼（建议关 TRACKING）

// --- GC9D01 屏 160×160 + SPI（esp_lcd）---
// ESP32-S3 默认：BL=35 DC=36 CS=TFT1_CS CLK=38 MOSI=39 RST=40
#define LCD_WIDTH        160
#define LCD_HEIGHT       160
#if CONFIG_IDF_TARGET_ESP32C3
// 双屏共享 SPI + 杜邦线：80MHz 会花屏/不显示，本板稳定上限 40MHz
#define LCD_SPI_HZ       60000000
#else
#define LCD_SPI_HZ       80000000
#endif
// 背光有效电平：多数屏高电平点亮；若不亮可改为 LOW
#define LCD_BACKLIGHT_ON HIGH

#if CONFIG_IDF_TARGET_ESP32S3
#define LCD_PIN_BL       35
#define LCD_PIN_DC       36
#define LCD_PIN_RST      40
#define LCD_PIN_MOSI     39
#define LCD_PIN_SCLK     38
#define LCD_USE_SPI3_HOST 1
#elif CONFIG_IDF_TARGET_ESP32C3
// 双屏共享 SPI：SCLK=GPIO4 MOSI=GPIO6
#define LCD_PIN_MOSI     6
#define LCD_PIN_SCLK     4
// 屏1：CS=7 DC=9 RST=3 BL=1
#define TFT1_CS            7
#define TFT1_DC            9
#define TFT1_RST           3
#define TFT1_BL            1
// 屏2：CS=8 DC=10 RST=2 BL=0
#define TFT2_CS            8
#define TFT2_DC           10
#define TFT2_RST           2
#define TFT2_BL            0
#define LCD_PIN_BL        TFT1_BL
#define LCD_PIN_DC        TFT1_DC
#define LCD_PIN_RST       TFT1_RST
#else
#define LCD_PIN_BL       35
#define LCD_PIN_DC       36
#define LCD_PIN_RST      40
#define LCD_PIN_MOSI     39
#define LCD_PIN_SCLK     38
#endif

#define BACKLIGHT_MAX        255
#define BACKLIGHT_BRIGHTNESS 100

// --- 屏上布局 / 片选 ---
#if CONFIG_IDF_TARGET_ESP32C3
#define TFT_COUNT          2
#define TFT_1_ROT          0
#define TFT_2_ROT          0   // 两块屏同向安装，勿用 3（会多转 90°）
#define EYE_1_XPOSITION    0
#define EYE_1_YPOSITION    0
#define EYE_2_XPOSITION    0
#define EYE_2_YPOSITION    0
#define NUM_EYES           2
#define DISPLAY_BACKLIGHT  -1   // 各眼独立背光，见 eyeInfo[].bl
#define BLINK_PIN          -1
#else
#define DISPLAY_BACKLIGHT    LCD_PIN_BL   // -1 关闭背光控制
#define TFT_COUNT          1
#define TFT1_CS            37
#define TFT2_CS            21
#define TFT_1_ROT          0
#define TFT_2_ROT          3
#define EYE_1_XPOSITION    0
#define EYE_1_YPOSITION    0
#define EYE_2_XPOSITION    0
#define EYE_2_YPOSITION    0
#define NUM_EYES           1
#define BLINK_PIN          47
#endif
#define LH_WINK_PIN        -1
#define RH_WINK_PIN        -1

// --- 眼球行为 ---
//#define JOYSTICK_X_PIN A0
//#define JOYSTICK_Y_PIN A1
#define TRACKING
#define AUTOBLINK

// 常态最小眼皮阈值（0=完全睁开，约 128=半闭）；160 屏略保留上眼皮更自然
#if LCD_WIDTH > SCREEN_WIDTH
#define EYELID_REST_U 88
#endif

#define LIGHT_CURVE        0.33
#define LIGHT_MIN          0
#define LIGHT_MAX          1023
#define IRIS_SMOOTH

// 平面色眼换虹膜色（catEye 等，把纹理里的纯色换成新颜色）
// RGB565 常用值：0xFFE0黄 0xF800红 0x07E0绿 0x001F蓝 0xFD20橙 0x780F紫
// 注释掉 IRIS_COLOR 则使用 catEye.h 里的原色
#define IRIS_COLOR       0x07E0
#define IRIS_COLOR_FROM  0xFFE0   // catEye 默认黄，一般不用改

#if !defined(IRIS_MIN)
#define IRIS_MIN           90
#endif
#if !defined(IRIS_MAX)
#define IRIS_MAX           130
#endif
