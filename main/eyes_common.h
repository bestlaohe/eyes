#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>

#define BUFFER_SIZE 1024

#ifdef USE_DMA
#define BUFFERS 2
#else
#define BUFFERS 1
#endif

typedef struct {
  int8_t select;
  int8_t wink;
  uint8_t rotation;
  int16_t xposition;
  int16_t yposition;
} eyeInfo_t;

#define NOBLINK 0
#define ENBLINK 1
#define DEBLINK 2

typedef struct {
  uint8_t state;
  uint32_t duration;
  uint32_t startTime;
} eyeBlink;

#include "config.h"

struct EyeState {
  int16_t tft_cs;
  eyeBlink blink;
  int16_t xposition;
  int16_t yposition;
};

extern EyeState eye[NUM_EYES];
extern TFT_eSPI tft;
extern uint16_t pbuffer[BUFFERS][BUFFER_SIZE];
extern bool dmaBuf;
extern uint32_t startTime;

extern void initEyes(void);
extern void updateEye(void);
extern void drawEye(uint8_t e, uint32_t iScale, uint32_t scleraX, uint32_t scleraY,
                    uint32_t uT, uint32_t lT);
extern void frame(uint16_t iScale);
extern void split(int16_t startValue, int16_t endValue, uint32_t startTime,
                  int32_t duration, int16_t range);

extern void user_setup(void);
extern void user_loop(void);
