#pragma once

#include "sdkconfig.h"

// BOE 0.71" GC9D01（160x160）— esp_lcd 显示配置
// ESP32-S3 默认引脚：BL=35  DC=36  CS=见 config.h  TFT1_CS  SCLK=38  MOSI=39  RST=40
// ESP32-C3 仅 SPI2(FSPI)，引脚请按实际板子修改下方 C3 区

#define LCD_WIDTH   160
#define LCD_HEIGHT  160

#define LCD_SPI_HZ  40000000

#if CONFIG_IDF_TARGET_ESP32S3
#define LCD_PIN_BL    35
#define LCD_PIN_DC    36
#define LCD_PIN_RST   40
#define LCD_PIN_MOSI  39
#define LCD_PIN_SCLK  38
#define LCD_USE_SPI3_HOST  1  // HSPI(SPI3)，与 Flash 所用 SPI2 分离
#endif

#if CONFIG_IDF_TARGET_ESP32C3
// C3 示例引脚（Lolin C3 mini 风格），接屏后请按接线修改
#define LCD_PIN_BL    3
#define LCD_PIN_DC    8
#define LCD_PIN_RST   10
#define LCD_PIN_MOSI  6
#define LCD_PIN_SCLK  4
#endif

#ifndef LCD_PIN_BL
#define LCD_PIN_BL    35
#define LCD_PIN_DC    36
#define LCD_PIN_RST   40
#define LCD_PIN_MOSI  39
#define LCD_PIN_SCLK  38
#endif

// 背光有效电平（与旧 TFT_eSPI 配置一致）
#define LCD_BACKLIGHT_ON HIGH
