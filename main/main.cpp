#include <Arduino.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "eyes_common.h"

static const char *TAG = "eyes";

EyeState eye[NUM_EYES];

uint32_t startTime; // 用于 FPS 计算
volatile uint32_t g_frame_count = 0;

static void fps_task(void *arg) {
  (void)arg;
  ESP_LOGI(TAG, "fps_task 已启动");
  uint32_t last_count = 0;
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(2000));
    const uint32_t count = g_frame_count;
    const uint32_t fps = (count - last_count) / 2;
    last_count = count;
    ESP_LOGI(TAG, "FPS: %lu (total=%lu)", (unsigned long)fps, (unsigned long)count);
  }
}


void setup(void) {
  initEyes();

  Serial.begin(115200);
  delay(100);
  ESP_LOGI(TAG, "boot [build: esp_lcd+fps100 COM12]");

#if defined(DISPLAY_BACKLIGHT) && (DISPLAY_BACKLIGHT >= 0)
  pinMode(DISPLAY_BACKLIGHT, OUTPUT);
  digitalWrite(DISPLAY_BACKLIGHT, LOW);
#endif

  user_setup();

  if (!display_init()) {
    ESP_LOGE(TAG, "display_init failed");
  }

#if defined(DISPLAY_BACKLIGHT) && (DISPLAY_BACKLIGHT >= 0)
  analogWrite(DISPLAY_BACKLIGHT, BACKLIGHT_BRIGHTNESS);
#endif

  randomSeed(esp_random());
  startTime = millis();

  if (xTaskCreatePinnedToCore(fps_task, "fps", 3072, nullptr, 1, nullptr, 0) != pdPASS) {
    ESP_LOGE(TAG, "fps_task 创建失败");
  }

  ESP_LOGI(TAG, "setup done");
}

void loop() {
  updateEye();
}
