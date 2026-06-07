#pragma once

#include "gif_catalog.h"

#include <stdint.h>

// source: vol2/00_0ad91561f033.gif
#define GIF_ACTIVE_CLIP_VOL "vol2"
#define GIF_ACTIVE_CLIP_VOL_NUM 2
#define GIF_ACTIVE_CLIP_INDEX 0
#define GIF_ACTIVE_CLIP_FILE "vol2/00_0ad91561f033.gif"

#define GIF_FRAME_W     66
#define GIF_FRAME_H     66
#define GIF_FRAME_COUNT 24
#define GIF_FRAME_MS    120
#define GIF_FRAME_STEP_USED 3
#define GIF_FRAME_FPS_x10 83
#define GIF_FRAME_BYTES (GIF_FRAME_W * GIF_FRAME_H * 2U)

extern uint16_t gif_frames[GIF_FRAME_COUNT][GIF_FRAME_W * GIF_FRAME_H];
