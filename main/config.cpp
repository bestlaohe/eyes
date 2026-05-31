#include "eyes_common.h"

#if (NUM_EYES == 2)
eyeInfo_t eyeInfo[] = {
    {TFT1_CS, LH_WINK_PIN, TFT_1_ROT, EYE_1_XPOSITION, EYE_1_YPOSITION},
    {TFT2_CS, RH_WINK_PIN, TFT_2_ROT, EYE_2_XPOSITION, EYE_2_YPOSITION},
};
#else
eyeInfo_t eyeInfo[] = {
    {TFT1_CS, LH_WINK_PIN, TFT_1_ROT, EYE_1_XPOSITION, EYE_1_YPOSITION},
};
#endif