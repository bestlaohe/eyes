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
#define HEART_COLOR        0xF800   // 红
#define HEART_GLOW_COLOR   0xA000   // 暗红柔光
#define HEART_BASE_RADIUS  48.0f    // 基准大小（像素）
#define HEART_GLOW_SCALE   1.18f    // 光晕外扩
#define HEART_BEAT_MS      850      // 一次心跳周期（毫秒）
#define HEART_BEAT_AMP     0.14f    // 第一拍幅度
#define HEART_BEAT_AMP2    0.07f    // 第二拍幅度
#define HEART_CENTER_X     0        // 微调中心（像素，正=右移）
#define HEART_CENTER_Y     0        // 微调中心（像素，正=下移）
