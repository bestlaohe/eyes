#include <Arduino.h>

#include "esp_log.h"

#include "config.h"
#include "display_port.h"
#include "heart.h"

static const char *TAG = "xintiao";

volatile uint32_t g_frame_count = 0;

void setup(void) {
  Serial.begin(115200);
  delay(100);
  ESP_LOGI(TAG, "boot [xintiao heartbeat COM5]");

#if defined(DISPLAY_BACKLIGHT) && (DISPLAY_BACKLIGHT >= 0)
  pinMode(DISPLAY_BACKLIGHT, OUTPUT);
  digitalWrite(DISPLAY_BACKLIGHT, LOW);
#endif

  if (!display_init()) {
    ESP_LOGE(TAG, "display_init failed");
    return;
  }

#if defined(DISPLAY_BACKLIGHT) && (DISPLAY_BACKLIGHT >= 0)
  analogWrite(DISPLAY_BACKLIGHT, BACKLIGHT_BRIGHTNESS);
#endif

  heart_init();
  ESP_LOGI(TAG, "setup done");
}

void loop() {
  heart_update();
  g_frame_count++;
}
