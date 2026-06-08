#pragma once

#include "sdkconfig.h"

// =============================================================================
// GIF 播放器 + GC9D01 160×160
// =============================================================================

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

#define TFT1_CS            37
#define TFT_1_ROT          0
#define EYE_1_XPOSITION    0
#define EYE_1_YPOSITION    0
#define NUM_EYES           1
#define LH_WINK_PIN        -1

// --- GIF 动图源：assets/salary_cat（改宏后 idf.py build 自动生成帧数据）---
#include "data/gif_catalog.h"

#define GIF_CLIP_VOL       2          // 1/2/3 = vol1/vol2/vol3
#define GIF_CLIP_INDEX     GIF_VOL2_00   
// vol2：GIF_CLIP_VOL 2  +  GIF_CLIP_INDEX GIF_VOL2_00
// vol3：GIF_CLIP_VOL 3  +  GIF_CLIP_INDEX GIF_VOL3_00

#define GIF_DRAW_W         66
#define GIF_DRAW_H         66
#define GIF_FRAME_STEP     1
#define GIF_FRAMES_IN_PSRAM 0
#define GIF_MAX_DRAM_KB    260
#define GIF_FPS_LOG_MS     1000
#define GIF_CLEAR_PAD      10        // 每帧绘制前向四周多清一圈，避免边缘残影
