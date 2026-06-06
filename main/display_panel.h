#pragma once

#include <stdint.h>

typedef struct {
  int8_t cs_pin;
  int8_t wink_pin;
  uint8_t rotation;
  int16_t xoffset;
  int16_t yoffset;
} display_panel_t;

extern display_panel_t display_panels[];
