#include "simd_math.h"

void simd_clear_block_stereo(float *out_l, float *out_r) {
    for (int i = 0; i < 8; ++i) {
        out_l[i] = 0.0f;
        out_r[i] = 0.0f;
    }
}

void simd_grain_accumulate_stereo(float *out_l, float *out_r, const float *audio, const float *window, uint32_t count, float gain_l, float gain_r) {
    for (uint32_t i = 0; i < count; ++i) {
        float processed = audio[i] * window[i];
        out_l[i] += processed * gain_l;
        out_r[i] += processed * gain_r;
    }
}

void simd_lerp_block(float *dest, const float *a, const float *b, float frac, uint32_t count) {
    for (uint32_t i = 0; i < count; ++i) {
        dest[i] = a[i] * (1.0f - frac) + b[i] * frac;
    }
}

void simd_adsr_process_block(float *values, const uint32_t *states, float attack_step, float decay_step, float sustain, float release_step, uint32_t count) {
    for (uint32_t i = 0; i < count; ++i) {
        switch (states[i]) {
            case 1: // ATTACK
                values[i] += attack_step;
                if (values[i] >= 1.0f) values[i] = 1.0f;
                break;
            case 2: // DECAY
                values[i] -= decay_step;
                if (values[i] <= sustain) values[i] = sustain;
                break;
            case 3: // SUSTAIN
                values[i] = sustain;
                break;
            case 4: // RELEASE
                values[i] -= release_step;
                if (values[i] <= 0.0f) values[i] = 0.0f;
                break;
            default:
                break;
        }
    }
}