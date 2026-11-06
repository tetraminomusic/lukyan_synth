#include "simd_math.h"

void simd_grain_accumulate(float *accum, const float *audio, const float *window, uint32_t count, float gain) {
    for (uint32_t i = 0; i < count; ++i) {
        accum[i] += audio[i] * window[i] * gain;
    }
}
