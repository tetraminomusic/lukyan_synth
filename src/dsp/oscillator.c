#include "oscillator.h"
#include <math.h>

#define PI 3.14159265358979323846

static float sample_buffers[NUM_WAVEFORMS][SAMPLE_BUFFER_SIZE];

void oscillator_init_buffers(double sample_rate) {
    float sr = (sample_rate > 0.0) ? (float)sample_rate : 44100.0f;
    float num_cycles = roundf((float)SAMPLE_BUFFER_SIZE * BASE_ROOT_FREQ / sr);

    for (int i = 0; i < SAMPLE_BUFFER_SIZE; ++i) {
        float phase = 2.0f * (float)PI * num_cycles * ((float)i / (float)SAMPLE_BUFFER_SIZE);

        sample_buffers[0][i] = sinf(phase) * 0.70f + sinf(phase * 2.0f) * 0.15f;

        float tri = 0.0f;
        for (int k = 0; k < 8; ++k) {
            float n = 2.0f * (float)k + 1.0f;
            float sign = (k % 2 == 0) ? 1.0f : -1.0f;
            tri += (sign / (n * n)) * sinf(phase * n);
        }
        sample_buffers[1][i] = tri * 0.70f;

        float saw = 0.0f;
        for (int k = 1; k <= 20; ++k) {
            saw += (1.0f / (float)k) * sinf(phase * (float)k);
        }
        sample_buffers[2][i] = saw * 0.45f;

        float sqr = 0.0f;
        for (int k = 0; k < 10; ++k) {
            float n = 2.0f * (float)k + 1.0f;
            sqr += (1.0f / n) * sinf(phase * n);
        }
        sample_buffers[3][i] = sqr * 0.50f;
    }
}

float oscillator_read_morphed(float pos, float morph) {
    int t0 = (int)morph;
    if (t0 > 2) t0 = 2;
    int t1 = t0 + 1;
    float frac_m = morph - (float)t0;
    if (morph >= 3.0f) {
        t0 = 3;
        t1 = 3;
        frac_m = 0.0f;
    }

    int idx_a = (int)pos;
    int idx_b = (idx_a + 1) % SAMPLE_BUFFER_SIZE;
    float frac_p = pos - (float)idx_a;

    float val0 = sample_buffers[t0][idx_a] * (1.0f - frac_p) + sample_buffers[t0][idx_b] * frac_p;
    float val1 = sample_buffers[t1][idx_a] * (1.0f - frac_p) + sample_buffers[t1][idx_b] * frac_p;

    return val0 * (1.0f - frac_m) + val1 * frac_m;
}
