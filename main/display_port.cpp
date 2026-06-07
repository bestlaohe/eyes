#include "display_port.h"

#include <string.h>

#include "eyes_common.h"

#include "driver/gpio.h"
#include "esp32-hal-ledc.h"
#include "driver/spi_master.h"
#include "esp_check.h"
#include "esp_err.h"
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

// 整帧一次 SPI 传输，减少分块开销（160×160×2 ≈ 50KB）
static constexpr int kBlitBandRows = LCD_HEIGHT;
static const size_t kBandBytes =
    (size_t)LCD_WIDTH * (size_t)kBlitBandRows * sizeof(uint16_t);

static esp_lcd_panel_handle_t s_panel[NUM_EYES];
static esp_lcd_panel_io_handle_t s_io[NUM_EYES];
static bool s_spi_bus_ready = false;

static volatile bool s_color_done[NUM_EYES];
static uint16_t *s_dma_buf = nullptr;

static bool IRAM_ATTR on_color_trans_done(esp_lcd_panel_io_handle_t panel_io,
                                          esp_lcd_panel_io_event_data_t *edata,
                                          void *user_ctx) {
  (void)panel_io;
  (void)edata;
  const uint8_t eye_index = (uint8_t)(uintptr_t)user_ctx;
  if (eye_index < NUM_EYES) {
    s_color_done[eye_index] = true;
  }
  return false;
}

static void display_wait_tx_done(uint8_t eye_index) {
  const uint32_t start = millis();
  while (!s_color_done[eye_index]) {
    if (millis() - start > 500) {
      ESP_LOGW(TAG, "eye%u SPI 传输超时", eye_index);
      s_color_done[eye_index] = true;
      break;
    }
    yield();
  }
}

static void display_backlight_apply(int8_t pin, uint8_t level) {
  if (pin < 0) {
    return;
  }

  const gpio_num_t gpio = (gpio_num_t)pin;
  const bool on_level   = (LCD_BACKLIGHT_ON == HIGH);

  if (level == 0) {
    ledcDetach((uint8_t)pin);
    gpio_set_direction(gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio, on_level ? 0 : 1);
    return;
  }

  if (level >= BACKLIGHT_MAX) {
    ledcDetach((uint8_t)pin);
    gpio_set_direction(gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio, on_level ? 1 : 0);
    return;
  }

  uint32_t duty = level;
  if (!on_level) {
    duty = (uint32_t)BACKLIGHT_MAX - level;
  }

  if (!ledcAttach((uint8_t)pin, 5000, 8)) {
    gpio_set_direction(gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio, (duty > (BACKLIGHT_MAX / 2)) ? (on_level ? 1 : 0)
                                                      : (on_level ? 0 : 1));
    ESP_LOGW(TAG, "GPIO%d LEDC 失败，改用数字背光 %s", pin, duty > (BACKLIGHT_MAX / 2) ? "开" : "关");
    return;
  }
  ledcWrite((uint8_t)pin, duty);
}

void display_backlight_init(void) {
  for (uint8_t e = 0; e < NUM_EYES; e++) {
    const int8_t bl = eyeInfo[e].bl;
    if (bl < 0) {
      continue;
    }
    display_backlight_apply(bl, BACKLIGHT_BRIGHTNESS);
    ESP_LOGI(TAG,
             "背光 eye%u GPIO%d 亮度=%u 有效电平=%s",
             e,
             bl,
             BACKLIGHT_BRIGHTNESS,
             (LCD_BACKLIGHT_ON == HIGH) ? "HIGH" : "LOW");
  }
}

static void display_log_pinout(void) {
#if CONFIG_IDF_TARGET_ESP32C3
  ESP_LOGI(TAG, "IO: 共享 SPI MOSI=GPIO%d SCLK=GPIO%d", LCD_PIN_MOSI, LCD_PIN_SCLK);
  for (uint8_t e = 0; e < NUM_EYES; e++) {
    ESP_LOGI(TAG,
             "  眼%u CS=GPIO%d DC=GPIO%d RST=GPIO%d BL=GPIO%d",
             e,
             eyeInfo[e].select,
             eyeInfo[e].dc,
             eyeInfo[e].rst,
             eyeInfo[e].bl);
  }
#endif
}

static bool display_ensure_dma_buf(void) {
  if (!s_dma_buf) {
    s_dma_buf = (uint16_t *)heap_caps_aligned_alloc(4, kBandBytes,
                                                    MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (!s_dma_buf) {
      ESP_LOGW(TAG, "DMA 对齐缓冲失败，尝试普通内部 RAM");
      s_dma_buf = (uint16_t *)heap_caps_malloc(kBandBytes, MALLOC_CAP_INTERNAL);
    }
    if (s_dma_buf) {
      ESP_LOGI(TAG, "刷屏缓冲 %u B @%p", (unsigned)kBandBytes, s_dma_buf);
    } else {
      ESP_LOGE(TAG, "刷屏缓冲分配失败");
    }
  }
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
  const int8_t cs_pin = eyeInfo[eye_index].select;
  if (cs_pin < 0) {
    return ESP_ERR_INVALID_ARG;
  }

  esp_lcd_panel_io_spi_config_t io_config = {};
  io_config.dc_gpio_num = eyeInfo[eye_index].dc;
  io_config.cs_gpio_num = cs_pin;
  io_config.pclk_hz = LCD_SPI_HZ;
  io_config.lcd_cmd_bits = 8;
  io_config.lcd_param_bits = 8;
  io_config.spi_mode = 0;
  io_config.trans_queue_depth = 4;
  io_config.on_color_trans_done = on_color_trans_done;
  io_config.user_ctx            = (void *)(uintptr_t)eye_index;

  ESP_RETURN_ON_ERROR(
      esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_HOST, &io_config, &s_io[eye_index]),
      TAG, "panel io create failed");

  esp_lcd_panel_dev_config_t panel_config = {};
  panel_config.reset_gpio_num = eyeInfo[eye_index].rst;
  panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
  panel_config.bits_per_pixel = 16;

  ESP_RETURN_ON_ERROR(
      esp_lcd_new_panel_gc9d01(s_io[eye_index], &panel_config, &s_panel[eye_index]),
      TAG, "panel create failed");

  ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(s_panel[eye_index]), TAG, "panel reset failed");
  ESP_RETURN_ON_ERROR(esp_lcd_panel_init(s_panel[eye_index]), TAG, "panel init failed");
  ESP_RETURN_ON_ERROR(esp_lcd_panel_disp_on_off(s_panel[eye_index], true), TAG, "panel on failed");

  display_apply_rotation(s_panel[eye_index], eyeInfo[eye_index].rotation);
  return ESP_OK;
}

bool display_init(void) {
  display_log_pinout();

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
    s_color_done[e] = true;
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

void display_fill_color(uint8_t eye_index, uint16_t rgb565) {
  if (eye_index >= NUM_EYES || !s_panel[eye_index] || !s_dma_buf) {
    return;
  }

  const size_t pixels = (size_t)LCD_WIDTH * (size_t)kBlitBandRows;
  for (size_t i = 0; i < pixels; i++) {
    s_dma_buf[i] = rgb565;
  }

  for (int16_t y = 0; y < LCD_HEIGHT; y += kBlitBandRows) {
    const int16_t h = (int16_t)((y + kBlitBandRows <= LCD_HEIGHT) ? kBlitBandRows
                                                                  : (LCD_HEIGHT - y));
    display_blit_rgb565(eye_index, 0, y, LCD_WIDTH, h, s_dma_buf);
  }
}

void display_fill_black(uint8_t eye_index) {
  display_fill_color(eye_index, 0x0000);
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

  // GC9D01 SPI 期望 RGB565 高字节在前
  const size_t count = (size_t)w * (size_t)h;
  for (size_t i = 0; i < count; i++) {
    s_dma_buf[i] = __builtin_bswap16(s_dma_buf[i]);
  }

  s_color_done[eye_index] = false;
  const esp_err_t err =
      esp_lcd_panel_draw_bitmap(s_panel[eye_index], x, y, x + w, y + h, s_dma_buf);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "eye%u draw_bitmap %dx%d@%d,%d: %s", eye_index, w, h, x, y, esp_err_to_name(err));
    s_color_done[eye_index] = true;
    return;
  }
  display_wait_tx_done(eye_index);
}
