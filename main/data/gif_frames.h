#pragma once

#include "gif_catalog.h"

#include <stdint.h>

// source: vol1/56_e0fd6e9a768f.gif
#define GIF_ACTIVE_CLIP_INDEX 56

#define GIF_FRAME_W     66
#define GIF_FRAME_H     66
#define GIF_FRAME_COUNT 28
#define GIF_FRAME_MS    40
#define GIF_FRAME_STEP_USED 1
#define GIF_FRAME_FPS_x10 250
#define GIF_FRAME_BYTES (GIF_FRAME_W * GIF_FRAME_H * 2U)

extern uint16_t gif_frames[GIF_FRAME_COUNT][GIF_FRAME_W * GIF_FRAME_H];
