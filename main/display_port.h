#pragma once

#include <stdint.h>

#ifndef LCD_RGB565_SWAP
#define LCD_RGB565_SWAP 1
#endif

static inline uint16_t rgb565_to_panel(uint16_t c) {
#if LCD_RGB565_SWAP
  return (uint16_t)((c << 8) | (c >> 8));
#else
  return c;
#endif
}

bool display_init(void);
void display_backlight_init(void);
void display_fill_black(uint8_t eye_index);
void display_fill_color(uint8_t eye_index, uint16_t rgb565);

uint16_t *display_dma_strip(void);
int display_dma_strip_rows(void);

void display_blit_rgb565(uint8_t eye_index, int16_t x, int16_t y,
                         int16_t w, int16_t h, const uint16_t *pixels);
