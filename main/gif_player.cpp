#include "gif_player.h"

#include <Arduino.h>
#include <string.h>

#include "config.h"
#include "data/gif_catalog.h"
#include "data/gif_frames.h"
#include "display_port.h"
#include "esp_log.h"

static const char *TAG = "gif_player";

static int s_anim_frame        = 0;
static uint32_t s_last_step_ms = 0;
static uint32_t s_fps_log_ms   = 0;
static uint32_t s_draw_count   = 0;

#if defined(GIF_ACTIVE_CLIP_VOL_NUM) && (GIF_CLIP_VOL != GIF_ACTIVE_CLIP_VOL_NUM)
#warning "config.h GIF_CLIP_VOL 与帧数据不一致，请重新 idf.py build"
#endif
#if defined(GIF_ACTIVE_CLIP_INDEX) && (GIF_CLIP_INDEX != GIF_ACTIVE_CLIP_INDEX)
#warning "config.h GIF_CLIP_INDEX 与帧数据不一致，请重新 idf.py build"
#endif

static void gif_player_clear_stage(void) {
  const int32_t pad = (int32_t)GIF_CLEAR_PAD;
  int32_t ox        = ((int32_t)LCD_WIDTH - (int32_t)GIF_FRAME_W) / 2 - pad;
  int32_t oy        = ((int32_t)LCD_HEIGHT - (int32_t)GIF_FRAME_H) / 2 - pad;
  int32_t cw        = (int32_t)GIF_FRAME_W + pad * 2;
  int32_t ch        = (int32_t)GIF_FRAME_H + pad * 2;

  if (ox < 0) {
    cw += ox;
    ox = 0;
  }
  if (oy < 0) {
    ch += oy;
    oy = 0;
  }
  if (ox + cw > (int32_t)LCD_WIDTH) {
    cw = (int32_t)LCD_WIDTH - ox;
  }
  if (oy + ch > (int32_t)LCD_HEIGHT) {
    ch = (int32_t)LCD_HEIGHT - oy;
  }
  if (cw > 0 && ch > 0) {
    display_fill_rect(0, (int16_t)ox, (int16_t)oy, (int16_t)cw, (int16_t)ch, 0);
  }
}

static void gif_player_draw(int frame_idx) {
  uint16_t *strip = display_dma_strip();
  if (!strip || frame_idx < 0 || frame_idx >= GIF_FRAME_COUNT) {
    return;
  }

  gif_player_clear_stage();

  const uint16_t *frame = gif_frames[frame_idx];
  const int32_t ox       = ((int32_t)LCD_WIDTH - (int32_t)GIF_FRAME_W) / 2;
  const int32_t oy       = ((int32_t)LCD_HEIGHT - (int32_t)GIF_FRAME_H) / 2;
  const int bandRows     = display_dma_strip_rows();
  const size_t row_bytes = (size_t)GIF_FRAME_W * sizeof(uint16_t);

  for (int32_t bandY = 0; bandY < (int32_t)GIF_FRAME_H; bandY += bandRows) {
    const int32_t bandH =
        (bandY + bandRows <= (int32_t)GIF_FRAME_H) ? bandRows
                                                   : ((int32_t)GIF_FRAME_H - bandY);
    memcpy(strip, frame + (size_t)bandY * (size_t)GIF_FRAME_W, row_bytes * (size_t)bandH);
    display_blit_rgb565(0,
                        (int16_t)ox,
                        (int16_t)(oy + bandY),
                        (int16_t)GIF_FRAME_W,
                        (int16_t)bandH,
                        strip);
  }
  s_draw_count++;
}

static void gif_player_log_fps(uint32_t now) {
#if GIF_FPS_LOG_MS > 0
  const uint32_t elapsed = now - s_fps_log_ms;
  if (elapsed < (uint32_t)GIF_FPS_LOG_MS) {
    return;
  }
  const float sec = (float)elapsed / 1000.0f;
  const float fps = sec > 0.0f ? ((float)s_draw_count / sec) : 0.0f;
  ESP_LOGI(TAG,
           "fps %.1f (target %.1f, frame %d/%d)",
           fps,
           (float)GIF_FRAME_FPS_x10 / 10.0f,
           s_anim_frame + 1,
           GIF_FRAME_COUNT);
  s_draw_count = 0;
  s_fps_log_ms = now;
#endif
}

void gif_player_init(void) {
  s_anim_frame   = 0;
  s_last_step_ms = millis();
  s_fps_log_ms   = s_last_step_ms;
  s_draw_count   = 0;
  gif_player_draw(0);
  ESP_LOGI(TAG,
           "%s clip %d: %s, %d frames %dx%d, step %d, %dms, %.1f fps",
           GIF_ACTIVE_CLIP_VOL,
           GIF_ACTIVE_CLIP_INDEX,
           GIF_ACTIVE_CLIP_FILE,
           GIF_FRAME_COUNT,
           GIF_FRAME_W,
           GIF_FRAME_H,
           GIF_FRAME_STEP_USED,
           GIF_FRAME_MS,
           (float)GIF_FRAME_FPS_x10 / 10.0f);
}

void gif_player_update(void) {
  const uint32_t now = millis();
  gif_player_log_fps(now);

  if (now - s_last_step_ms < (uint32_t)GIF_FRAME_MS) {
    return;
  }
  s_last_step_ms = now;

  s_anim_frame = (s_anim_frame + 1) % GIF_FRAME_COUNT;
  gif_player_draw(s_anim_frame);
}
