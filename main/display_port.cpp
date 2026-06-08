#include "display_port.h"

#include <Arduino.h>
#include <string.h>

#include "config.h"
#include "display_panel.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_gc9d01.h"

static const char *TAG = "display";

#if CONFIG_IDF_TARGET_ESP32S3 && defined(LCD_USE_SPI3_HOST)
#define LCD_SPI_HOST SPI3_HOST
#else
#define LCD_SPI_HOST SPI2_HOST
#endif

static constexpr int kBlitBandRows = 64;
static const size_t kBandBytes =
    (size_t)LCD_WIDTH * (size_t)kBlitBandRows * sizeof(uint16_t);

static esp_lcd_panel_handle_t s_panel[NUM_EYES];
static esp_lcd_panel_io_handle_t s_io[NUM_EYES];
static bool s_spi_bus_ready = false;

static volatile bool s_color_done = true;
static uint16_t s_dma_buf_storage[LCD_WIDTH * kBlitBandRows];
static uint16_t *s_dma_buf = s_dma_buf_storage;

static bool IRAM_ATTR on_color_trans_done(esp_lcd_panel_io_handle_t panel_io,
                                          esp_lcd_panel_io_event_data_t *edata,
                                          void *user_ctx) {
  (void)panel_io;
  (void)edata;
  (void)user_ctx;
  s_color_done = true;
  return false;
}

static void display_wait_tx_done(void) {
  const uint32_t start = millis();
  while (!s_color_done) {
    if (millis() - start > 100) {
      s_color_done = true;
      break;
    }
    yield();
  }
}

static bool display_ensure_dma_buf(void) {
  return s_dma_buf != nullptr;
}

uint16_t *display_dma_strip(void) {
  return display_ensure_dma_buf() ? s_dma_buf : nullptr;
}

int display_dma_strip_rows(void) {
  return kBlitBandRows;
}

static void display_apply_rotation(esp_lcd_panel_handle_t panel, uint8_t rotation) {
  switch (rotation % 4) {
    case 0:
      esp_lcd_panel_swap_xy(panel, false);
      esp_lcd_panel_mirror(panel, false, false);
      break;
    case 1:
      esp_lcd_panel_swap_xy(panel, true);
      esp_lcd_panel_mirror(panel, false, true);
      break;
    case 2:
      esp_lcd_panel_swap_xy(panel, false);
      esp_lcd_panel_mirror(panel, true, true);
      break;
    default:
      esp_lcd_panel_swap_xy(panel, true);
      esp_lcd_panel_mirror(panel, true, false);
      break;
  }
}

static esp_err_t display_panel_add(uint8_t eye_index) {
  const int8_t cs_pin = display_panels[eye_index].cs_pin;
  if (cs_pin < 0) {
    return ESP_ERR_INVALID_ARG;
  }

  esp_lcd_panel_io_spi_config_t io_config = {};
  io_config.dc_gpio_num = LCD_PIN_DC;
  io_config.cs_gpio_num = cs_pin;
  io_config.pclk_hz = LCD_SPI_HZ;
  io_config.lcd_cmd_bits = 8;
  io_config.lcd_param_bits = 8;
  io_config.spi_mode = 0;
  io_config.trans_queue_depth = 4;
  io_config.on_color_trans_done = on_color_trans_done;

  ESP_RETURN_ON_ERROR(
      esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_HOST, &io_config, &s_io[eye_index]),
      TAG, "panel io create failed");

  esp_lcd_panel_dev_config_t panel_config = {};
  panel_config.reset_gpio_num = LCD_PIN_RST;
  panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
  panel_config.bits_per_pixel = 16;

  ESP_RETURN_ON_ERROR(
      esp_lcd_new_panel_gc9d01(s_io[eye_index], &panel_config, &s_panel[eye_index]),
      TAG, "panel create failed");

  ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(s_panel[eye_index]), TAG, "panel reset failed");
  ESP_RETURN_ON_ERROR(esp_lcd_panel_init(s_panel[eye_index]), TAG, "panel init failed");
  ESP_RETURN_ON_ERROR(esp_lcd_panel_disp_on_off(s_panel[eye_index], true), TAG, "panel on failed");

  display_apply_rotation(s_panel[eye_index], display_panels[eye_index].rotation);
  return ESP_OK;
}

bool display_init(void) {
  if (!display_ensure_dma_buf()) {
    Serial.println("display: DMA 缓冲分配失败");
    return false;
  }

  if (!s_spi_bus_ready) {
    spi_bus_config_t bus_config = {};
    bus_config.mosi_io_num = LCD_PIN_MOSI;
    bus_config.miso_io_num = -1;
    bus_config.sclk_io_num = LCD_PIN_SCLK;
    bus_config.quadwp_io_num = -1;
    bus_config.quadhd_io_num = -1;
    bus_config.max_transfer_sz = (int)kBandBytes;

    if (spi_bus_initialize(LCD_SPI_HOST, &bus_config, SPI_DMA_CH_AUTO) != ESP_OK) {
      Serial.println("display: SPI 总线初始化失败");
      return false;
    }
    s_spi_bus_ready = true;
  }

  for (uint8_t e = 0; e < NUM_EYES; e++) {
    if (display_panel_add(e) != ESP_OK) {
      Serial.print("display: 面板 #");
      Serial.print(e);
      Serial.println(" 初始化失败");
      return false;
    }
    display_fill_black(e);
  }

  ESP_LOGI(TAG, "esp_lcd ready SPI %lu MHz band %d rows",
           (unsigned long)(LCD_SPI_HZ / 1000000UL), kBlitBandRows);
  return true;
}

void display_fill_rect(uint8_t eye_index, int16_t x, int16_t y,
                       int16_t w, int16_t h, uint16_t color) {
  if (eye_index >= NUM_EYES || !s_panel[eye_index] || !s_dma_buf || w <= 0 || h <= 0) {
    return;
  }

  for (int16_t bandY = 0; bandY < h; bandY += kBlitBandRows) {
    const int16_t bandH =
        (int16_t)((bandY + kBlitBandRows <= h) ? kBlitBandRows : (h - bandY));
    const size_t count = (size_t)w * (size_t)bandH;
    if (color == 0) {
      memset(s_dma_buf, 0, count * sizeof(uint16_t));
    } else {
      for (size_t i = 0; i < count; i++) {
        s_dma_buf[i] = color;
      }
    }
    display_blit_rgb565(eye_index, x, (int16_t)(y + bandY), w, bandH, s_dma_buf);
  }
}

void display_fill_black(uint8_t eye_index) {
  display_fill_rect(eye_index, 0, 0, LCD_WIDTH, LCD_HEIGHT, 0);
}

void display_blit_rgb565(uint8_t eye_index, int16_t x, int16_t y,
                         int16_t w, int16_t h, const uint16_t *pixels) {
  if (eye_index >= NUM_EYES || !s_panel[eye_index] || !pixels || w <= 0 || h <= 0) {
    return;
  }
  if (!s_dma_buf) {
    return;
  }

  const size_t bytes = (size_t)w * (size_t)h * sizeof(uint16_t);
  if (bytes > kBandBytes) {
    ESP_LOGE(TAG, "blit %dx%d exceeds band %d rows", w, h, kBlitBandRows);
    return;
  }

  if (pixels != s_dma_buf) {
    memcpy(s_dma_buf, pixels, bytes);
  }

  // GC9D01 SPI 期望 RGB565 高字节在前；与 Animated Eyes 原版 pushPixels 一致
  const size_t count = (size_t)w * (size_t)h;
  for (size_t i = 0; i < count; i++) {
    const uint16_t c = s_dma_buf[i];
    s_dma_buf[i] = (uint16_t)((c << 8) | (c >> 8));
  }

  s_color_done = false;
  esp_lcd_panel_draw_bitmap(s_panel[eye_index], x, y, x + w, y + h, s_dma_buf);
  display_wait_tx_done();
}
