#ifndef _TFT_CONFIG_H_
#define _TFT_CONFIG_H_

// BOE 0.71" GC9D01（160x160）
// ESP32-S3：DMA 须用 HSPI(SPI3)；MISO=-1 在 HSPI 下合法（FSPI 会误映射 MOSI 导致 init 卡死）
// 引脚：BL=35  DC=36  CS=37  CL=38  DA=39  RST=40

#define USER_SETUP_ID 471

#define GC9D01_DRIVER
#define USE_HSPI_PORT

#define TFT_WIDTH  160
#define TFT_HEIGHT 160

#define TFT_CS   37
#define TFT_MOSI 39
#define TFT_SCLK 38
#define TFT_DC   36
#define TFT_RST  40
#define TFT_BL   35
#define TFT_MISO -1   // 显示-only SPI，DMA 勿与 MOSI 共用 MISO
#define TOUCH_CS -1

#define TFT_BACKLIGHT_ON HIGH  // 背光有效电平

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT

#define SPI_FREQUENCY       40000000  // SPI 写入时钟（Hz）
#define SPI_READ_FREQUENCY  40000000  // SPI 读取时钟（Hz）
#define SPI_TOUCH_FREQUENCY 2500000   // 触摸 SPI 时钟（Hz）

#endif // _TFT_CONFIG_H_
