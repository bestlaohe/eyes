#include <Arduino.h>

#include "eyes_common.h"

TFT_eSPI tft;  // 一个实例用于 1 个或 2 个显示器

// 在眼睛渲染期间使用像素缓冲区
uint16_t pbuffer[BUFFERS][BUFFER_SIZE];
bool dmaBuf = 0;  // DMA 双缓冲切换标志

EyeState eye[NUM_EYES];

uint32_t startTime;  // 用于 FPS 计算



static void backlight_off(void) {

#if defined(DISPLAY_BACKLIGHT) && (DISPLAY_BACKLIGHT >= 0)

  pinMode(DISPLAY_BACKLIGHT, OUTPUT);

  digitalWrite(DISPLAY_BACKLIGHT, LOW);

#endif

}



static void backlight_on(void) {

#if defined(DISPLAY_BACKLIGHT) && (DISPLAY_BACKLIGHT >= 0)

  pinMode(DISPLAY_BACKLIGHT, OUTPUT);

  digitalWrite(DISPLAY_BACKLIGHT, HIGH);

  analogWrite(DISPLAY_BACKLIGHT, BACKLIGHT_MAX);

#endif

}



// 初始化 -- 启动时运行一次
void setup(void) {

  Serial.begin(115200);




  // 背光先关闭，避免上电白屏被照亮
  backlight_off();



  user_setup();

  initEyes();



  Serial.println("初始化显示器");

#if defined(TFT_RST) && (TFT_RST >= 0)

  // 硬件复位显示屏
  pinMode(TFT_RST, OUTPUT);

  digitalWrite(TFT_RST, HIGH);

  delay(50);

  digitalWrite(TFT_RST, LOW);

  delay(50);

  digitalWrite(TFT_RST, HIGH);

  delay(120);

#endif

  tft.init();
  Serial.println("初始化显示器2");

  // tft.init() 会通过 TFT_BL 自动开背光，清屏前先关掉
  backlight_off();

  Serial.println("初始化显示器3");

#ifdef USE_DMA

  tft.initDMA();

#endif

Serial.println("初始化显示器6");

  // 将片选引脚拉高，以便可以单独配置各块显示器
  digitalWrite(eye[0].tft_cs, HIGH);

  if (NUM_EYES > 1) digitalWrite(eye[1].tft_cs, HIGH);



  for (uint8_t e = 0; e < NUM_EYES; e++) {

    digitalWrite(eye[e].tft_cs, LOW);

    tft.setRotation(eyeInfo[e].rotation);

    tft.fillScreen(TFT_BLACK);

    digitalWrite(eye[e].tft_cs, HIGH);

  }



  Serial.printf("固件 %s %s\n", __DATE__, __TIME__);
  Serial.println("背光已打开，显示初始化完成");

  backlight_on();

  startTime = millis();  // 记录起始时间，供 FPS 统计
}



// 主循环 -- setup() 后持续运行
void loop(void) {
  updateEye();
}
