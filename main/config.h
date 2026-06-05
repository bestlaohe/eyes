#pragma once

#include "sdkconfig.h"

// =============================================================================
// 项目配置（只改本文件即可）
// =============================================================================

// --- 眼睛外观：只保留一个 #include ---
#define SYMMETRICAL_EYELID

//#include "data/defaultEye.h"
//#include "data/dragonEye.h"
//#include "data/noScleraEye.h"
//#include "data/goatEye.h"
//#include "data/newtEye.h"
//#include "data/terminatorEye.h"
#include "data/catEye.h"
//#include "data/owlEye.h"      // 建议配合 EYE_NO_TRACKING（头文件内已定义）
//#include "data/naugaEye.h"
//#include "data/doeEye.h"

// --- GC9D01 屏 160×160 + SPI（esp_lcd）---
// ESP32-S3 默认：BL=35 DC=36 CS=TFT1_CS CLK=38 MOSI=39 RST=40
#define LCD_WIDTH        160
#define LCD_HEIGHT       160
#define LCD_SPI_HZ       80000000
#define LCD_BACKLIGHT_ON HIGH

#if CONFIG_IDF_TARGET_ESP32S3
#define LCD_PIN_BL       35
#define LCD_PIN_DC       36
#define LCD_PIN_RST      40
#define LCD_PIN_MOSI     39
#define LCD_PIN_SCLK     38
#define LCD_USE_SPI3_HOST 1
#elif CONFIG_IDF_TARGET_ESP32C3
#define LCD_PIN_BL       3
#define LCD_PIN_DC       8
#define LCD_PIN_RST      10
#define LCD_PIN_MOSI     6
#define LCD_PIN_SCLK     4
#else
#define LCD_PIN_BL       35
#define LCD_PIN_DC       36
#define LCD_PIN_RST      40
#define LCD_PIN_MOSI     39
#define LCD_PIN_SCLK     38
#endif

#define DISPLAY_BACKLIGHT    LCD_PIN_BL   // -1 关闭背光控制
#define BACKLIGHT_MAX        255
#define BACKLIGHT_BRIGHTNESS 50

// --- 屏上布局 / 片选 ---
#define TFT_COUNT          1
#define TFT1_CS            37
#define TFT2_CS            21
#define TFT_1_ROT          0
#define TFT_2_ROT          3
#define EYE_1_XPOSITION    16
#define EYE_1_YPOSITION    16
#define EYE_2_XPOSITION    0
#define EYE_2_YPOSITION    0

#define NUM_EYES           1

#define BLINK_PIN          47
#define LH_WINK_PIN        -1
#define RH_WINK_PIN        -1

// --- 眼球行为 ---
//#define JOYSTICK_X_PIN A0
//#define JOYSTICK_Y_PIN A1
#define TRACKING
#define AUTOBLINK

#define LIGHT_CURVE        0.33
#define LIGHT_MIN          0
#define LIGHT_MAX          1023
#define IRIS_SMOOTH

#if !defined(IRIS_MIN)
#define IRIS_MIN           90
#endif
#if !defined(IRIS_MAX)
#define IRIS_MAX           130
#endif
