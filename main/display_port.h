#pragma once

#include <stdint.h>

// 初始化 SPI 总线与各眼 GC9D01 面板（esp_lcd + DMA）
bool display_init(void);

// 清屏为黑色
void display_fill_black(uint8_t eye_index);

// 绘制 RGB565 矩形（x/y 为屏坐标，pixels 行优先，w*h 个 uint16_t）
void display_blit_rgb565(uint8_t eye_index, int16_t x, int16_t y,
                         int16_t w, int16_t h, const uint16_t *pixels);
