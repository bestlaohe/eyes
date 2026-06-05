#pragma once

#include <Arduino.h>

// 每只眼睛的硬件配置（见 config.cpp 中的 eyeInfo[]）
typedef struct {
  int8_t select;      // 片选引脚
  int8_t wink;        // 单眼 wink 按钮引脚（-1 表示无）
  uint8_t rotation;   // 屏幕旋转 0-3
  int16_t xposition;  // 眼睛图像 X 偏移
  int16_t yposition;  // 眼睛图像 Y 偏移
} eyeInfo_t;

#include "config.h"
#include "display_port.h"

// 眨眼状态机
#define NOBLINK 0   // 未眨眼
#define ENBLINK 1   // 正在闭眼
#define DEBLINK 2   // 正在睁眼

typedef struct {
  uint8_t state;       // 当前状态
  uint32_t duration;   // 状态持续时间（微秒）
  uint32_t startTime;  // 进入当前状态的时刻（微秒）
} eyeBlink;

// 运行时每只眼睛的状态
struct EyeState {
  eyeBlink blink;     // 眨眼状态
  int16_t xposition;  // 渲染 X 偏移
  int16_t yposition;  // 渲染 Y 偏移
};

extern EyeState eye[NUM_EYES];
extern uint32_t startTime;
extern volatile uint32_t g_frame_count;

extern void initEyes(void);
extern void updateEye(void);
extern void drawEye(uint8_t e, uint32_t iScale, uint32_t scleraX, uint32_t scleraY,
                    uint32_t uT, uint32_t lT);
extern void frame(uint16_t iScale);

extern void user_setup(void);
extern void user_loop(void);
