#ifndef GRANULAR_H
#define GRANULAR_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_GRAINS 64
#define HANN_LUT_SIZE 2048
#define SAMPLE_BUFFER_SIZE 96000

typedef struct {
    bool active;
    float pos;
    float speed;
    float length;
    float progress;
} Grain;

typedef struct {
    double sample_rate;
    float frequency;
    bool is_note_on;
    float spawn_timer;

    float sample_buffer[SAMPLE_BUFFER_SIZE];
    float hann_lut[HANN_LUT_SIZE];
    Grain grains[MAX_GRAINS];
} GranularEngine;

void granular_init(GranularEngine *engine, double sample_rate);
void granular_set_frequency(GranularEngine *engine, float frequency);
void granular_set_gate(GranularEngine *engine, bool gate);
void granular_reset(GranularEngine *engine);
float granular_render_sample(GranularEngine *engine);

#endif