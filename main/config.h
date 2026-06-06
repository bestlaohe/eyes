#pragma once

#include "sdkconfig.h"

// =============================================================================
// 心跳模式配置（xintiao 分支）
// =============================================================================

// --- GC9D01 屏 160×160 + SPI（esp_lcd）---
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

#define DISPLAY_BACKLIGHT    LCD_PIN_BL
#define BACKLIGHT_MAX        255
#define BACKLIGHT_BRIGHTNESS 80

// --- 屏上布局 / 片选 ---
#define TFT1_CS            37
#define TFT_1_ROT          0
#define EYE_1_XPOSITION    0
#define EYE_1_YPOSITION    0
#define NUM_EYES           1
#define LH_WINK_PIN        -1

// --- 爱心跳动 ---
#define HEART_COLOR        0xF800   // 纯红
#define HEART_BASE_RADIUS  40.0f
#define HEART_AA_SIZE      0.045f   // 边缘抗锯齿
#define HEART_BEAT_MS      850
#define HEART_BEAT_AMP     0.16f    // 第一拍缩放幅度
#define HEART_BEAT_AMP2    0.08f    // 第二拍缩放幅度
#define HEART_CENTER_X     0
#define HEART_CENTER_Y     0
