#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "simd_math.h"

static inline uint64_t rdtsc() {
    unsigned int lo, hi;
    __asm__ volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

#define BLOCKS 10000
#define BLOCK_SIZE 8

int main() {
    float accum_c[SIMD_BLOCK_SIZE] = {0};
    float accum_asm[SIMD_BLOCK_SIZE] = {0};
    float audio[SIMD_BLOCK_SIZE] = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    float window[SIMD_BLOCK_SIZE] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    float gain = 0.5f;

    uint64_t start_c = rdtsc();
    for (int i = 0; i < BLOCKS; ++i) {
        simd_grain_accumulate_stereo(accum_c, accum_c, audio, window, SIMD_BLOCK_SIZE, gain, gain);
    }
    uint64_t end_c = rdtsc();

    uint64_t start_asm = rdtsc();
    for (int i = 0; i < BLOCKS; ++i) {
        simd_grain_accumulate_stereo(accum_asm, accum_asm, audio, window, SIMD_BLOCK_SIZE, gain, gain);
    }
    uint64_t end_asm = rdtsc();

    printf("Результаты оптимизации (Cycles per %d blocks):\n", BLOCKS);
    printf("Си-реализация:   %llu циклов\n", (end_c - start_c));
    printf("Ассемблер/SIMD:  %llu циклов\n", (end_asm - start_asm));
    printf("Прирост скорости: %.2fx\n", (double)(end_c - start_c) / (double)(end_asm - start_asm));

    return 0;
}