#include "simd_math.h"
#include <math.h>

#define PI 3.14159265358979323846

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

static float ic1eq_l = 0.0f, ic2eq_l = 0.0f;
static float ic1eq_r = 0.0f, ic2eq_r = 0.0f;

void simd_svf_process_stereo(float *samples_l, float *samples_r, uint32_t count, float cutoff_hz, float resonance, float sample_rate) {
    float sr = (sample_rate > 0.0f) ? sample_rate : 44100.0f;
    float g = tanf((float)PI * cutoff_hz / sr);
    float k = 1.0f / resonance;
    float a1 = 1.0f / (1.0f + g * (g + k));
    float a2 = g * a1;
    float a3 = g * a2;

    for (uint32_t i = 0; i < count; ++i) {
        // Левый канал
        float v3_l = samples_l[i] - ic2eq_l;
        float v1_l = a1 * ic1eq_l + a2 * v3_l;
        float v2_l = ic2eq_l + a2 * ic1eq_l + a3 * v3_l;
        ic1eq_l = 2.0f * v1_l - ic1eq_l;
        ic2eq_l = 2.0f * v2_l - ic2eq_l;
        samples_l[i] = v2_l;

        // Правый канал
        float v3_r = samples_r[i] - ic2eq_r;
        float v1_r = a1 * ic1eq_r + a2 * v3_r;
        float v2_r = ic2eq_r + a2 * ic1eq_r + a3 * v3_r;
        ic1eq_r = 2.0f * v1_r - ic1eq_r;
        ic2eq_r = 2.0f * v2_r - ic2eq_r;
        samples_r[i] = v2_r;
    }
}
