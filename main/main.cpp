#include <Arduino.h>

#include "esp_log.h"

#include "config.h"
#include "display_port.h"
#include "gif_player.h"

static const char *TAG = "eyes";

void setup(void) {
  Serial.begin(115200);
  delay(100);
  ESP_LOGI(TAG, "boot [gif player]");

#if defined(DISPLAY_BACKLIGHT) && (DISPLAY_BACKLIGHT >= 0)
  pinMode(DISPLAY_BACKLIGHT, OUTPUT);
  analogWrite(DISPLAY_BACKLIGHT, BACKLIGHT_BRIGHTNESS);
#endif

  if (!display_init()) {
    ESP_LOGE(TAG, "display_init failed");
    return;
  }

  gif_player_init();
  ESP_LOGI(TAG, "setup done");
}

void loop() {
  gif_player_update();
}
