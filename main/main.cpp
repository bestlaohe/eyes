#include <Arduino.h>

#include "eyes_common.h"

EyeState eye[NUM_EYES];

uint32_t startTime; // 用于 FPS 计算


void setup(void) {
  Serial.begin(115200);
  Serial.println("开始");

#if defined(DISPLAY_BACKLIGHT) && (DISPLAY_BACKLIGHT >= 0)
  Serial.println("关闭背光");
  pinMode(DISPLAY_BACKLIGHT, OUTPUT);
  digitalWrite(DISPLAY_BACKLIGHT, LOW);
#endif

  user_setup();
  initEyes();

  Serial.println("初始化显示器 (esp_lcd)");
  if (!display_init()) {
    Serial.println("显示器初始化失败");
  }

#if defined(DISPLAY_BACKLIGHT) && (DISPLAY_BACKLIGHT >= 0)
  Serial.println("背光现在打开！");
  analogWrite(DISPLAY_BACKLIGHT, BACKLIGHT_BRIGHTNESS);
#endif

  randomSeed(esp_random());
  startTime = millis();
  Serial.println("setup 完成，进入主循环");
}

void loop() {
  updateEye();
}
