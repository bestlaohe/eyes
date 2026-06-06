#pragma once

#include "salary_cat_vol1.h"

#include <stdint.h>

// source: vol1/00_76dec3740034.gif
#define SALARY_CAT_ACTIVE_CLIP_INDEX 0

#define SALARY_CAT_FRAME_W     80
#define SALARY_CAT_FRAME_H     80
#define SALARY_CAT_FRAME_COUNT 18
#define SALARY_CAT_FRAME_MS    40
#define SALARY_CAT_FRAME_STEP_USED 1
#define SALARY_CAT_FRAME_FPS_x10 250
#define SALARY_CAT_FRAME_BYTES (SALARY_CAT_FRAME_W * SALARY_CAT_FRAME_H * 2U)

extern uint16_t salary_cat_frames[SALARY_CAT_FRAME_COUNT][SALARY_CAT_FRAME_W * SALARY_CAT_FRAME_H];
