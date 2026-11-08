#ifndef SIMD_MATH_H
#define SIMD_MATH_H

#include <stdint.h>

void simd_clear_block_stereo(float *out_l, float *out_r);
void simd_grain_accumulate_stereo(float *out_l, float *out_r, const float *audio, const float *window, uint32_t count, float gain_l, float gain_r);
void simd_lerp_block(float *dest, const float *a, const float *b, float frac, uint32_t count);
void simd_adsr_process_block(float *values, const uint32_t *states, float attack_step, float decay_step, float sustain, float release_step, uint32_t count);

#endif