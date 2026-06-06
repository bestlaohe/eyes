#include <Arduino.h>

#include "esp_log.h"

#include "config.h"
#include "data/salaryCatFrames.h"
#include "display_port.h"
#include "salary_cat.h"

static const char *TAG = "yuexinmao";

volatile uint32_t g_frame_count = 0;

void setup(void) {
  Serial.begin(115200);
  delay(100);
  ESP_LOGI(TAG, "boot [salary cat gif]");

#if defined(DISPLAY_BACKLIGHT) && (DISPLAY_BACKLIGHT >= 0)
  pinMode(DISPLAY_BACKLIGHT, OUTPUT);
  analogWrite(DISPLAY_BACKLIGHT, BACKLIGHT_BRIGHTNESS);
#endif

  if (!display_init()) {
    ESP_LOGE(TAG, "display_init failed");
    return;
  }

  if (!salary_cat_preload()) {
    ESP_LOGE(TAG, "salary_cat_preload failed");
    return;
  }

  salary_cat_init();
  ESP_LOGI(TAG, "setup done");
}

void loop() {
  salary_cat_update();
  g_frame_count++;
}
