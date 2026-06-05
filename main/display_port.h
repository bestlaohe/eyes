#pragma once

#include <stdint.h>

bool display_init(void);
void display_fill_black(uint8_t eye_index);

uint16_t *display_dma_strip(void);
int display_dma_strip_rows(void);

void display_blit_rgb565(uint8_t eye_index, int16_t x, int16_t y,
                         int16_t w, int16_t h, const uint16_t *pixels);
