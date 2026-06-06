#include "salary_cat.h"

#include <Arduino.h>
#include <string.h>

#include "config.h"
#include "data/salary_cat_vol1.h"
#include "data/salaryCatFrames.h"
#include "display_port.h"
#include "esp_log.h"

static const char *TAG = "salary_cat";

static int s_anim_frame        = 0;
static uint32_t s_last_step_ms = 0;
static bool s_ready            = false;

static void salary_cat_draw(int frame_idx) {
  uint16_t *strip = display_dma_strip();
  if (!strip || !s_ready || frame_idx < 0 || frame_idx >= SALARY_CAT_FRAME_COUNT) {
    return;
  }

  const uint16_t *frame = salary_cat_frames[frame_idx];
  const int32_t ox       = ((int32_t)LCD_WIDTH - (int32_t)SALARY_CAT_FRAME_W) / 2;
  const int32_t oy       = ((int32_t)LCD_HEIGHT - (int32_t)SALARY_CAT_FRAME_H) / 2;
  const int bandRows     = display_dma_strip_rows();
  const size_t row_bytes = (size_t)SALARY_CAT_FRAME_W * sizeof(uint16_t);

  for (int32_t bandY = 0; bandY < (int32_t)SALARY_CAT_FRAME_H; bandY += bandRows) {
    const int32_t bandH =
        (bandY + bandRows <= (int32_t)SALARY_CAT_FRAME_H) ? bandRows
                                                          : ((int32_t)SALARY_CAT_FRAME_H - bandY);
    memcpy(strip, frame + (size_t)bandY * (size_t)SALARY_CAT_FRAME_W, row_bytes * (size_t)bandH);
    display_blit_rgb565(0,
                        (int16_t)ox,
                        (int16_t)(oy + bandY),
                        (int16_t)SALARY_CAT_FRAME_W,
                        (int16_t)bandH,
                        strip);
  }
}

bool salary_cat_preload(void) {
  s_ready = true;
  return true;
}

void salary_cat_init(void) {
  if (!s_ready) {
    return;
  }

  s_anim_frame   = 0;
  s_last_step_ms = millis();
  salary_cat_draw(0);
  ESP_LOGI(TAG,
           "vol1 clip %d/%d %s — %d frames %dx%d @ %dms",
           SALARY_CAT_ACTIVE_CLIP_INDEX,
           SALARY_CAT_VOL1_COUNT,
           SALARY_CAT_VOL1_PATHS[SALARY_CAT_ACTIVE_CLIP_INDEX],
           SALARY_CAT_FRAME_COUNT,
           SALARY_CAT_FRAME_W,
           SALARY_CAT_FRAME_H,
           SALARY_CAT_FRAME_MS);
}

void salary_cat_update(void) {
  if (!s_ready) {
    return;
  }

  const uint32_t now = millis();
  if (now - s_last_step_ms < (uint32_t)SALARY_CAT_FRAME_MS) {
    return;
  }
  s_last_step_ms = now;

  s_anim_frame = (s_anim_frame + 1) % SALARY_CAT_FRAME_COUNT;
  salary_cat_draw(s_anim_frame);
}
