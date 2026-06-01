#pragma once

// 在这里选择的Pin基于Teensy 3.x项目的原始Adafruit Learning System指南。

// 图形设置（眼睛的外观） -----------------------------------

// 如果使用单眼，你可能希望启用下一行，它使用一个更简单的“橄榄球形状”的眼睛，左右对称。
// 默认形状包括泪车，形成明显的左右眼。

#define SYMMETRICAL_EYELID // 对称眼皮

// 启用以下其中一个#include -- 各种眼睛的大量图形表：
//#include "data/defaultEye.h"      // 标准的类似人类的榛果色眼睛 -或-
//#include "data/dragonEye.h"     // 炽热的龙/恶魔眼睛，瞳孔裂缝 -或-
//#include "data/noScleraEye.h"   // 大虹膜，无眼珠 -或-
//#include "data/goatEye.h"       // 水平瞳孔的山羊/克拉慕斯之眼 -或-
//#include "data/newtEye.h"       // 火蜥蜴之眼 -或-
//#include "data/terminatorEye.h" // 终结者之眼!快上直升机！
#include "data/catEye.h"        // 卡通猫眼（平面”2D“颜色）
//#include "data/owlEye.h"        // 室内猫眼（禁用追踪）
//#include "data/naugaEye.h"      // Nauga的滚动眼睛（禁用追踪）
//#include "data/doeEye.h"        // 卡通鹿眼（禁用追踪）

// 显示器硬件设置（BOE 0.71" GC9D01 160x160） -------------------
// BL=35  DC=36  CS=37  CL=38  DA=39  RST=40
#define TFT_COUNT 1        // 屏幕数量（1或2）
#define TFT1_CS 37         // TFT 1片选引脚（CS，设为-1来使用TFT_eSPI setup）
#define TFT2_CS 21         // TFT 2片选引脚（设为-1来使用TFT_eSPI setup）
#define TFT_1_ROT 0        // TFT 1旋转
#define TFT_2_ROT 3        // TFT 2旋转
#define EYE_1_XPOSITION  16 // 128x128 眼睛在 160x160 屏上居中
#define EYE_1_YPOSITION  16
#define EYE_2_XPOSITION  0
#define EYE_2_YPOSITION  0

#define DISPLAY_BACKLIGHT  35 // 背光 BL（-1表示没有）
#define BACKLIGHT_MAX    255  // 最大背光（0-255）
// 若背光仍不亮，在 screen_config.h 把 TFT_BACKLIGHT_ON 改为 LOW（低电平有效背光）

// 眼睛列表 ----------------------------------------------------------------
#define NUM_EYES 1 // 要展示的眼睛数量（1或2）

#define BLINK_PIN   47 // 手动眨眼按钮引脚（控制双眼）
#define LH_WINK_PIN -1 // 左Wink引脚（设为-1表示没有引脚）
#define RH_WINK_PIN -1 // 右Wink引脚（设为-1表示没有引脚）

// 下面这个表格包含每只眼睛的一行。这个表格必须以这个名称存在，并且必须包含一行或者多行。每一行包含三个项目：
// 对应于TFT/OLED屏幕的选择线的引脚号，该眼睛的“wink”按钮的引脚号（或者-1，如果不使用），屏幕的旋转值（0-3）和眼睛的x位置偏移量。

#if (NUM_EYES == 2)
extern eyeInfo_t eyeInfo[];
#else
extern eyeInfo_t eyeInfo[];
#endif

// 输入设置（用于控制眼睛动作） -----------------------------

// JOYSTICK_X_PIN和JOYSTICK_Y_PIN指定用于手动控制眼睛的模拟摇杆的模拟输入引脚。如果设置为-1或者没有定义，
// 眼睛将自动移动。
// IRIS_PIN指定一个模拟输入引脚，用于照明栅使瞳孔反应光（或电位器用于手动控制）。如果设置为-1或者没有定义，
// 瞳孔将自行改变。
// BLINK_PIN指定一个输入引脚，按钮（接地）将使任何/所有的眼睛闪烁。如果设置为-1或者没有定义，眼睛将只有在定义了AUTOBLINK，
// 或者上面的eyeInfo[]表中包括了每只眼睛的wink按钮设置时才会闪烁。

//#define JOYSTICK_X_PIN A0 // 眼睛水平位置的模拟引脚（其他为自动）
//#define JOYSTICK_Y_PIN A1 // 眼睛垂直位置的模拟引脚（其他为自动）
//#define JOYSTICK_X_FLIP   // 如果定义，反转摇杆X轴
//#define JOYSTICK_Y_FLIP   // 如果定义，反转摇杆Y轴
#define TRACKING            // 如果定义，眼睑跟踪瞳孔
#define AUTOBLINK           // 如果定义，眼睛还会自动眨眼

//  #define LIGHT_PIN      -1 // 光线传感器引脚
  #define LIGHT_CURVE  0.33 // 光线传感器调整曲线
  #define LIGHT_MIN       0 // 光线传感器的最小有效读数
  #define LIGHT_MAX    1023 // 传感器的最大有效读数

#define IRIS_SMOOTH         // 如果启用，从IRIS_PIN过滤输入
#if !defined(IRIS_MIN)      // 每只眼睛可能有自己的最小/最大
  #define IRIS_MIN       90 // 在最亮的光线下的虹膜大小（0-1023）
#endif
#if !defined(IRIS_MAX)
  #define IRIS_MAX      130 // 在最黑暗的光线下的虹膜大小（0-1023）
#endif