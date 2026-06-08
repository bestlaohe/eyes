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
#undef IRIS_MIN
#undef IRIS_MAX
#define IRIS_MIN           18     // 最亮：再小(≈16)会看不见瞳孔
#define IRIS_MAX           34     // 最暗：实测刚好合适
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
#define TFT_COUNT          1
#define TFT_1_ROT          0
#define TFT_2_ROT          0
#define EYE_1_XPOSITION    0
#define EYE_1_YPOSITION    0
#define EYE_2_XPOSITION    0
#define EYE_2_YPOSITION    0
#define NUM_EYES           1
#define DISPLAY_BACKLIGHT  TFT1_BL
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

// --- 光敏电阻 GT36516（与 10kΩ 组成分压，控制瞳孔）---
// 接法：3.3V — 光敏 — GPIO — 10kΩ — GND（光敏与 10k 并联时视情况用 LIGHT_PIN_FLIP）
#if CONFIG_IDF_TARGET_ESP32C3
// C3 仅 GPIO0~4 有 ADC；光敏焊在 GPIO8 时，用杜邦线把 GPIO8 与 GPIO0 短接即可
#define LIGHT_PIN          0      // analogRead 用这根（ADC）
#define LIGHT_PIN_BRIDGE   8      // 光敏实际焊盘；高阻 INPUT，不驱动，与 GPIO0 短接
#else
#define LIGHT_PIN          8
#endif
// 亮→瞳孔小、暗→瞳孔大（与真人一致）。若方向反了再取消下行注释：
// #define LIGHT_PIN_FLIP
#define LIGHT_LOG_MS       1000   // 串口打印光敏 ADC，0=关闭

// 常态最小眼皮阈值（0=完全睁开，约 128=半闭）；160 屏略保留上眼皮更自然
#if LCD_WIDTH > SCREEN_WIDTH
#define EYELID_REST_U 88
#endif

#define LIGHT_ADC_MAX      4095   // ESP32 Arduino analogRead 为 12 位
#define LIGHT_CURVE        2.5     // >1 拉大亮暗对比，瞳孔变化更明显
#define LIGHT_MIN          0
#define LIGHT_MAX          LIGHT_ADC_MAX
// 把实测 raw 区间拉伸到满量程（按串口 raw 改，覆盖你手电/遮光范围）
#define LIGHT_CAL_LO       2000
#define LIGHT_CAL_HI       4100
// #define IRIS_SMOOTH      // 光敏控瞳孔时关闭平滑，跟手更快

// 平面色眼换虹膜色（catEye 等，把纹理里的纯色换成新颜色）
// RGB565 常用值：0xFFE0黄 0xF800红 0x07E0绿 0x001F蓝 0xFD20橙 0x780F紫
// 注释掉 IRIS_COLOR 则使用 catEye.h 里的原色
#define IRIS_COLOR       0x07E0
#define IRIS_COLOR_FROM  0xFFE0   // catEye 默认黄，一般不用改

