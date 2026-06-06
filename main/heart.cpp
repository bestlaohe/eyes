#include "heart.h"

#include <Arduino.h>
#include <math.h>

#include "display_port.h"
#include "esp_log.h"

#include "config.h"

static const char *TAG = "heart";

static float s_centroid_x = 0.0f;
static float s_centroid_y = 0.0f;

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

static void heart_map_pixel(int32_t px, int32_t py, float radius, float *hx, float *hy) {
  const float scr_cx =
      ((float)LCD_WIDTH - 1.0f) * 0.5f + (float)HEART_CENTER_X;
  const float scr_cy =
      ((float)LCD_HEIGHT - 1.0f) * 0.5f + (float)HEART_CENTER_Y;

  *hx = s_centroid_x + ((float)px - scr_cx) / radius;
  *hy = s_centroid_y - ((float)py - scr_cy) / radius;
}

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

static float clampf(float v, float lo, float hi) {
  if (v < lo) {
    return lo;
  }
  if (v > hi) {
    return hi;
  }
  return v;
}

static uint16_t lerp_rgb565(uint16_t c0, uint16_t c1, float t) {
  t = clampf(t, 0.0f, 1.0f);
  const uint8_t r0 = (uint8_t)((c0 >> 11) & 0x1F);
  const uint8_t g0 = (uint8_t)((c0 >> 5) & 0x3F);
  const uint8_t b0 = (uint8_t)(c0 & 0x1F);
  const uint8_t r1 = (uint8_t)((c1 >> 11) & 0x1F);
  const uint8_t g1 = (uint8_t)((c1 >> 5) & 0x3F);
  const uint8_t b1 = (uint8_t)(c1 & 0x1F);
  const uint8_t r  = (uint8_t)((float)r0 + ((float)r1 - (float)r0) * t);
  const uint8_t g  = (uint8_t)((float)g0 + ((float)g1 - (float)g0) * t);
  const uint8_t b  = (uint8_t)((float)b0 + ((float)b1 - (float)b0) * t);
  return (uint16_t)(((uint16_t)r << 11) | ((uint16_t)g << 5) | (uint16_t)b);
}

// 4 点采样抗锯齿；内部纯色，亮度不随心跳变化
static uint16_t heart_pixel_color(float hx, float hy) {
  const float o = HEART_AA_SIZE * 0.35f;
  float cov     = 0.0f;
  cov += heart_inside(hx - o, hy - o) ? 1.0f : 0.0f;
  cov += heart_inside(hx + o, hy - o) ? 1.0f : 0.0f;
  cov += heart_inside(hx - o, hy + o) ? 1.0f : 0.0f;
  cov += heart_inside(hx + o, hy + o) ? 1.0f : 0.0f;
  cov *= 0.25f;

  if (cov <= 0.0f) {
    return 0x0000;
  }
  if (cov >= 1.0f) {
    return HEART_COLOR;
  }
  return lerp_rgb565(0x0000, HEART_COLOR, cov);
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
  heart_draw(heart_beat_scale(millis()));
}
