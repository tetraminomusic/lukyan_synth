#include "fast_math.h"

float fast_tanhf(float x) {
    if (x < -3.0f) return -1.0f;
    if (x > 3.0f) return 1.0f;
    return x * (27.0f + x * x) / (27.0f + 9.0f * x * x);
}

float fast_pow2(float p) {
    extern float powf(float, float);
    return powf(2.0f, p);
}