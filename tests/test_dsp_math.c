#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "fast_math.h"
#include "simd_math.h"

int main(void) {
    for (float x = -3.0f; x <= 3.0f; x += 0.1f) {
        float ref = tanhf(x);
        float approx = fast_tanhf(x);
        float diff = fabsf(ref - approx);
        assert(diff < 0.08f);
    }

    float block_l[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    float block_r[8] = {9, 10, 11, 12, 13, 14, 15, 16};
    simd_clear_block_stereo(block_l, block_r);
    for (int i = 0; i < 8; ++i) {
        assert(block_l[i] == 0.0f);
        assert(block_r[i] == 0.0f);
    }

    float a[8] = {0.0f, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f, 70.0f};
    float b[8] = {10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f, 70.0f, 80.0f};
    float dest[8];
    simd_lerp_block(dest, a, b, 0.5f, 8);
    for (int i = 0; i < 8; ++i) {
        float expected = (a[i] + b[i]) * 0.5f;
        assert(fabsf(dest[i] - expected) < 1e-5f);
    }

    printf("[PASSED] DSP Math & SIMD unit test: Precision and vector routines verified\n");
    return 0;
}