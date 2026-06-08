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
#if NUM_EYES > 1
    ESP_LOGI(TAG,
             "FPS: %lu (绘制调用/秒, 每只眼约 %lu Hz, total=%lu)",
             (unsigned long)fps,
             (unsigned long)(fps / NUM_EYES),
             (unsigned long)count);
#else
    ESP_LOGI(TAG,
             "FPS: %lu (屏刷新/秒, total=%lu)",
             (unsigned long)fps,
             (unsigned long)count);
#endif
  }
}


void setup(void) {
  initEyes();

  Serial.begin(115200);
  delay(100);
  ESP_LOGI(TAG, "boot [build: esp_lcd+fps100 COM12]");

  if (!display_init()) {
    ESP_LOGE(TAG, "display_init failed");
  } else {
    display_backlight_init();
  }
#if defined(DISPLAY_BACKLIGHT) && (DISPLAY_BACKLIGHT >= 0)
  pinMode(DISPLAY_BACKLIGHT, OUTPUT);
  digitalWrite(DISPLAY_BACKLIGHT, LCD_BACKLIGHT_ON);
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
