#include "heart.h"

#include <Arduino.h>
#include <math.h>
#include <string.h>

#include "display_port.h"
#include "esp_log.h"

#include "config.h"

static const char *TAG = "heart";

static float s_centroid_x = 0.0f;
static float s_centroid_y = 0.0f;

// 心形隐式方程：(x²+y²-1)³ - x²y³ ≤ 0
static bool heart_inside(float x, float y) {
  const float a = x * x + y * y - 1.0f;
  return (a * a * a - x * x * y * y * y) <= 0.0f;
}

static void heart_compute_centroid(void) {
  float sum_x = 0.0f;
  float sum_y = 0.0f;
  int count   = 0;

  for (int iy = -140; iy <= 140; iy++) {
    for (int ix = -140; ix <= 140; ix++) {
      const float x = (float)ix * 0.01f;
      const float y = (float)iy * 0.01f;
      if (heart_inside(x, y)) {
        sum_x += x;
        sum_y += y;
        count++;
      }
    }
  }

  if (count > 0) {
    s_centroid_x = sum_x / (float)count;
    s_centroid_y = sum_y / (float)count;
  }
}

// 屏幕像素 → 心形坐标（以屏幕中心为缩放原点）
static void heart_map_pixel(int32_t px, int32_t py, float radius, float *hx, float *hy) {
  const float scr_cx =
      ((float)LCD_WIDTH - 1.0f) * 0.5f + (float)HEART_CENTER_X;
  const float scr_cy =
      ((float)LCD_HEIGHT - 1.0f) * 0.5f + (float)HEART_CENTER_Y;

  *hx = s_centroid_x + ((float)px - scr_cx) / radius;
  *hy = s_centroid_y - ((float)py - scr_cy) / radius;
}

// 心跳缩放：双拍（咚-咚-停）
static float heart_beat_scale(uint32_t ms) {
  const uint32_t period = HEART_BEAT_MS;
  const uint32_t t      = ms % period;

  if (t < 90U) {
    return 1.0f + HEART_BEAT_AMP * sinf((float)t / 90.0f * 1.5707963f);
  }
  if (t < 160U) {
    return 1.0f + HEART_BEAT_AMP * cosf((float)(t - 90U) / 70.0f * 1.5707963f);
  }
  if (t < 230U) {
    return 1.0f + HEART_BEAT_AMP2 * sinf((float)(t - 160U) / 70.0f * 1.5707963f);
  }
  if (t < 290U) {
    return 1.0f + HEART_BEAT_AMP2 * cosf((float)(t - 230U) / 60.0f * 1.5707963f);
  }
  return 1.0f;
}

static uint16_t heart_pixel_color(float hx, float hy) {
  if (heart_inside(hx, hy)) {
    return HEART_COLOR;
  }

  const float tx = s_centroid_x + (hx - s_centroid_x) / HEART_GLOW_SCALE;
  const float ty = s_centroid_y + (hy - s_centroid_y) / HEART_GLOW_SCALE;
  if (heart_inside(tx, ty)) {
    return HEART_GLOW_COLOR;
  }
  return 0x0000;
}

static void heart_draw(float scale) {
  uint16_t *strip = display_dma_strip();
  if (!strip) {
    return;
  }

  const int bandRows = display_dma_strip_rows();
  const float radius = HEART_BASE_RADIUS * scale;

  for (int32_t bandY = 0; bandY < (int32_t)LCD_HEIGHT; bandY += bandRows) {
    const int32_t bandH = (int32_t)((bandY + bandRows <= (int32_t)LCD_HEIGHT)
                                        ? bandRows
                                        : ((int32_t)LCD_HEIGHT - bandY));
    for (int32_t sy = 0; sy < bandH; sy++) {
      const int32_t py = bandY + sy;
      uint16_t *row    = &strip[(size_t)sy * LCD_WIDTH];
      for (int32_t px = 0; px < (int32_t)LCD_WIDTH; px++) {
        float hx, hy;
        heart_map_pixel(px, py, radius, &hx, &hy);
        row[px] = heart_pixel_color(hx, hy);
      }
    }
    display_blit_rgb565(0,
                         EYE_1_XPOSITION,
                         (int16_t)(EYE_1_YPOSITION + bandY),
                         LCD_WIDTH,
                         (int16_t)bandH,
                         strip);
  }
}

void heart_init(void) {
  heart_compute_centroid();
  ESP_LOGI(TAG,
           "heartbeat init %dx%d centroid=(%.3f, %.3f)",
           LCD_WIDTH,
           LCD_HEIGHT,
           s_centroid_x,
           s_centroid_y);
}

void heart_update(void) {
  const float scale = heart_beat_scale(millis());
  heart_draw(scale);
}
