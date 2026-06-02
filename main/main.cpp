#include <Arduino.h>
#include "esp_heap_caps.h"

#include "eyes_common.h"

TFT_eSPI tft; // 一个实例用于 1 个或 2 个显示器

// DMA 缓冲区须位于 DMA 可访问内存（ESP32-S3）
uint16_t *pbuffer[BUFFERS] = {nullptr, nullptr};
bool dmaBuf = 0; // DMA 双缓冲切换标志

EyeState eye[NUM_EYES];

uint32_t startTime; // 用于 FPS 计算


void setup(void) {
  Serial.begin(115200);

#if defined(DISPLAY_BACKLIGHT) && (DISPLAY_BACKLIGHT >= 0)
  // 启用背光引脚，最初关闭
  Serial.println("关闭背光");
  pinMode(DISPLAY_BACKLIGHT, OUTPUT);
  digitalWrite(DISPLAY_BACKLIGHT, LOW);
#endif

  // 用户调用其他功能
  user_setup();

  // 初始化眼睛，这将为tft.init()设置所有芯片选择为低电平
  initEyes();
  Serial.println("initEyes done");

#ifdef USE_DMA
  for (int i = 0; i < BUFFERS; i++) {
    pbuffer[i] = (uint16_t *)heap_caps_malloc(BUFFER_SIZE * sizeof(uint16_t),
                                              MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (!pbuffer[i]) {
      Serial.println("ERR: DMA pixel buffer alloc failed");
    }
  }
#endif

  Serial.println("初始化显示器");
  tft.init();

#ifdef USE_DMA
  if (!tft.initDMA()) {
    Serial.println("WARN: initDMA failed");
  } else {
    Serial.println("initDMA bus OK");
  }
#endif

  // 将芯片选择引脚设置为高电平，以便可以单独配置显示器
  digitalWrite(eye[0].tft_cs, HIGH);
  if (NUM_EYES > 1) digitalWrite(eye[1].tft_cs, HIGH);

  for (uint8_t e = 0; e < NUM_EYES; e++) {
    digitalWrite(eye[e].tft_cs, LOW);
    tft.setRotation(eyeInfo[e].rotation);
    tft.fillScreen(TFT_BLACK);
    digitalWrite(eye[e].tft_cs, HIGH);
  }

#if defined(DISPLAY_BACKLIGHT) && (DISPLAY_BACKLIGHT >= 0)
  Serial.println("背光现在打开！");
  analogWrite(DISPLAY_BACKLIGHT, BACKLIGHT_MAX);
#endif

  startTime = millis();  // 用于帧速率计算
}

// 主循环 -- 在setup()后持续运行 ----------------------------
#ifdef USE_DMA
static void dma_enable_task(void *) {
  vTaskDelay(pdMS_TO_TICKS(500));
  if (tft.enableDmaEngine()) {
    Serial.println("initDMA OK");
  } else {
    Serial.println("WARN: enableDmaEngine failed");
  }
  vTaskDelete(nullptr);
}
#endif

void loop() {
#ifdef USE_DMA
  static bool dma_task_started = false;
  if (!dma_task_started && tft.DMA_Bus_Ready && !tft.DMA_Enabled) {
    dma_task_started = true;
    xTaskCreate(dma_enable_task, "dma_en", 4096, nullptr, 1, nullptr);
  }
#endif
  updateEye();
}