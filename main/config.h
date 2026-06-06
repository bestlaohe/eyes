#pragma once

#include "sdkconfig.h"

// =============================================================================
// 月薪猫 GIF 动画（yuexinmao 分支）
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
#define BACKLIGHT_BRIGHTNESS 35

// --- 屏上布局 / 片选 ---
#define TFT1_CS            37
#define TFT_1_ROT          0
#define EYE_1_XPOSITION    0
#define EYE_1_YPOSITION    0
#define NUM_EYES           1
#define LH_WINK_PIN        -1

// --- 月薪猫 vol1 表情包（assets/salary_cat/vol1，共 62 个）---
// 切换动画：改 SALARY_CAT_CLIP_INDEX 为 SALARY_CAT_VOL1_XX，再运行
//   python tools/build_salary_cat_clips.py
#include "data/salary_cat_vol1.h"

#ifndef SALARY_CAT_CLIP_INDEX
#define SALARY_CAT_CLIP_INDEX  SALARY_CAT_VOL1_00
#endif

#define SALARY_CAT_DRAW_W         80
#define SALARY_CAT_DRAW_H         80
#define SALARY_CAT_FRAME_STEP     1    // 1=原速(40ms)；内存不够时构建脚本会自动加大
#define SALARY_CAT_MAX_DRAM_KB    250  // 帧数据占用上限
#define SALARY_CAT_FPS_LOG_MS     1000 // 串口刷新实际帧率间隔

#define CAT_COLOR_BG           0x0000
