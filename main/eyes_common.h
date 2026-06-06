#pragma once

#include <Arduino.h>

#include <stdint.h>

typedef struct {
  int8_t select;
  int8_t wink;
  uint8_t rotation;
  int16_t xposition;
  int16_t yposition;
} eyeInfo_t;

#include "config.h"
#include "display_port.h"

extern eyeInfo_t eyeInfo[];
