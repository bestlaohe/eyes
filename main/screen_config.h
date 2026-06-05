#ifndef _TFT_CONFIG_H_
#define _TFT_CONFIG_H_

// TFT_eSPI 库编译用桥接（引脚与分辨率见 config.h，勿在此重复配置）
#include "config.h"

#define GC9D01_DRIVER

#if defined(LCD_USE_SPI3_HOST)
#define USE_HSPI_PORT
#endif

#define TFT_WIDTH            LCD_WIDTH
#define TFT_HEIGHT           LCD_HEIGHT
#define TFT_CS               TFT1_CS
#define TFT_MOSI             LCD_PIN_MOSI
#define TFT_SCLK             LCD_PIN_SCLK
#define TFT_DC               LCD_PIN_DC
#define TFT_RST              LCD_PIN_RST
#define TFT_BL               LCD_PIN_BL
#define TFT_BACKLIGHT_ON     LCD_BACKLIGHT_ON

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

#define SPI_FREQUENCY        LCD_SPI_HZ
#define SPI_READ_FREQUENCY   LCD_SPI_HZ
#define SPI_TOUCH_FREQUENCY  2500000

#endif
