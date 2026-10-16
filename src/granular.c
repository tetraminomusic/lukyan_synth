#include "granular.h"
#include <math.h>
#include <stdlib.h>

#define PI 3.14159265358979323846

static void init_hann_lut(GranularEngine *engine) {
    for (int i = 0; i < HANN_LUT_SIZE; ++i) {
        engine->hann_lut[i] = 0.5f * (1.0f - cosf((2.0f * (float)PI * (float)i) / (float)(HANN_LUT_SIZE - 1)));
    }
}

static void init_sample_buffer(GranularEngine *engine) {
    for (int i = 0; i < SAMPLE_BUFFER_SIZE; ++i) {
        float t = (float)i / (float)SAMPLE_BUFFER_SIZE;
        float wave = sinf(2.0f * (float)PI * 220.0f * t) * 0.5f
                   + sinf(2.0f * (float)PI * 440.0f * t) * 0.25f
                   + sinf(2.0f * (float)PI * 880.0f * t) * 0.125f;
        engine->sample_buffer[i] = wave;
    }
}

void granular_init(GranularEngine *engine, double sample_rate) {
    engine->sample_rate = sample_rate;
    engine->frequency = 440.0f;
    engine->is_note_on = false;
    engine->spawn_timer = 0.0f;

    init_hann_lut(engine);
    init_sample_buffer(engine);

    for (int i = 0; i < MAX_GRAINS; ++i) {
        engine->grains[i].active = false;
    }
}

void granular_set_frequency(GranularEngine *engine, float frequency) {
    engine->frequency = frequency;
}

void granular_set_gate(GranularEngine *engine, bool gate) {
    engine->is_note_on = gate;
}

void granular_reset(GranularEngine *engine) {
    engine->is_note_on = false;
    engine->spawn_timer = 0.0f;
    for (int i = 0; i < MAX_GRAINS; ++i) {
        engine->grains[i].active = false;
    }
}

static void spawn_grain(GranularEngine *engine) {
    for (int i = 0; i < MAX_GRAINS; ++i) {
        if (!engine->grains[i].active) {
            engine->grains[i].active = true;
            engine->grains[i].pos = (float)(rand() % (SAMPLE_BUFFER_SIZE / 2));
            engine->grains[i].speed = engine->frequency / 220.0f;
            engine->grains[i].length = (float)engine->sample_rate * 0.08f;
            engine->grains[i].progress = 0.0f;
            break;
        }
    }
}

float granular_render_sample(GranularEngine *engine) {
    const float spawn_interval = (engine->sample_rate > 0.0) ? ((float)engine->sample_rate / 40.0f) : 1000.0f;

    if (engine->is_note_on) {
        engine->spawn_timer += 1.0f;
        if (engine->spawn_timer >= spawn_interval) {
            engine->spawn_timer = 0.0f;
            spawn_grain(engine);
        }
    }

    float mixed_sample = 0.0f;

    for (int g = 0; g < MAX_GRAINS; ++g) {
        if (!engine->grains[g].active) continue;

        Grain *grain = &engine->grains[g];

        float win_norm = grain->progress / grain->length;
        int lut_idx = (int)(win_norm * (float)(HANN_LUT_SIZE - 1));
        if (lut_idx >= HANN_LUT_SIZE) lut_idx = HANN_LUT_SIZE - 1;
        float env = engine->hann_lut[lut_idx];

        int idx_a = (int)grain->pos;
        int idx_b = (idx_a + 1) % SAMPLE_BUFFER_SIZE;
        float frac = grain->pos - (float)idx_a;
        float audio_val = engine->sample_buffer[idx_a] * (1.0f - frac) + engine->sample_buffer[idx_b] * frac;

        mixed_sample += audio_val * env * 0.15f;

        grain->pos += grain->speed;
        if (grain->pos >= (float)SAMPLE_BUFFER_SIZE) {
            grain->pos -= (float)SAMPLE_BUFFER_SIZE;
        }

        grain->progress += 1.0f;
        if (grain->progress >= grain->length) {
            grain->active = false;
        }
    }

    return mixed_sample;
}