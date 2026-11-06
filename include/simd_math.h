#ifndef SIMD_MATH_H
#define SIMD_MATH_H

#include <stdint.h>

void simd_grain_accumulate(float *accum, const float *audio, const float *window, uint32_t count, float gain);

#endif
