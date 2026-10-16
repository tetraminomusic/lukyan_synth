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
        float wave = sinf(2.0f * (float)PI * 110.0f * t) * 0.4f
                   + sinf(2.0f * (float)PI * 220.0f * t) * 0.25f
                   + sinf(2.0f * (float)PI * 330.0f * t) * 0.15f
                   + sinf(2.0f * (float)PI * 440.0f * t) * 0.1f;
        engine->sample_buffer[i] = wave;
    }
}

void granular_init(GranularEngine *engine, double sample_rate) {
    engine->sample_rate = sample_rate;
    engine->playhead = 0.0f;

    init_hann_lut(engine);
    init_sample_buffer(engine);

    for (int i = 0; i < MAX_GRAINS; ++i) {
        engine->grains[i].active = false;
    }

    for (int v = 0; v < MAX_VOICES; ++v) {
        engine->voices[v].active = false;
        engine->voices[v].key = -1;
        engine->voices[v].frequency = 440.0f;
        engine->voices[v].spawn_timer = 0.0f;
    }
}

void granular_note_on(GranularEngine *engine, int32_t key, float frequency) {
    int target_idx = -1;

    for (int v = 0; v < MAX_VOICES; ++v) {
        if (!engine->voices[v].active) {
            target_idx = v;
            break;
        }
    }

    if (target_idx == -1) {
        target_idx = 0;
    }

    engine->voices[target_idx].active = true;
    engine->voices[target_idx].key = key;
    engine->voices[target_idx].frequency = frequency;
    engine->voices[target_idx].spawn_timer = 0.0f;
}

void granular_note_off(GranularEngine *engine, int32_t key) {
    for (int v = 0; v < MAX_VOICES; ++v) {
        if (engine->voices[v].active && engine->voices[v].key == key) {
            engine->voices[v].active = false;
            engine->voices[v].key = -1;
        }
    }
}

void granular_reset(GranularEngine *engine) {
    for (int v = 0; v < MAX_VOICES; ++v) {
        engine->voices[v].active = false;
        engine->voices[v].key = -1;
        engine->voices[v].spawn_timer = 0.0f;
    }
    for (int i = 0; i < MAX_GRAINS; ++i) {
        engine->grains[i].active = false;
    }
    engine->playhead = 0.0f;
}

static void spawn_grain(GranularEngine *engine, float frequency) {
    for (int i = 0; i < MAX_GRAINS; ++i) {
        if (!engine->grains[i].active) {
            engine->grains[i].active = true;

            float jitter = ((float)(rand() % 2000) - 1000.0f);
            float start_pos = engine->playhead + jitter;
            if (start_pos < 0.0f) start_pos += (float)SAMPLE_BUFFER_SIZE;
            if (start_pos >= (float)SAMPLE_BUFFER_SIZE) start_pos -= (float)SAMPLE_BUFFER_SIZE;

            engine->grains[i].pos = start_pos;
            engine->grains[i].speed = frequency / 220.0f;
            engine->grains[i].length = (float)engine->sample_rate * 0.09f;
            engine->grains[i].progress = 0.0f;
            engine->grains[i].pan = (float)(rand() % 1000) / 1000.0f;
            break;
        }
    }
}

void granular_render_sample(GranularEngine *engine, float *out_l, float *out_r) {
    const float spawn_interval = (engine->sample_rate > 0.0) ? ((float)engine->sample_rate / 35.0f) : 1000.0f;

    for (int v = 0; v < MAX_VOICES; ++v) {
        if (!engine->voices[v].active) continue;

        engine->voices[v].spawn_timer += 1.0f;
        if (engine->voices[v].spawn_timer >= spawn_interval) {
            engine->voices[v].spawn_timer = 0.0f;
            spawn_grain(engine, engine->voices[v].frequency);
        }
    }

    engine->playhead += 0.5f;
    if (engine->playhead >= (float)SAMPLE_BUFFER_SIZE) {
        engine->playhead -= (float)SAMPLE_BUFFER_SIZE;
    }

    float mixed_l = 0.0f;
    float mixed_r = 0.0f;

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

        float grain_amp = audio_val * env * 0.08f;
        mixed_l += grain_amp * (1.0f - grain->pan);
        mixed_r += grain_amp * grain->pan;

        grain->pos += grain->speed;
        if (grain->pos >= (float)SAMPLE_BUFFER_SIZE) {
            grain->pos -= (float)SAMPLE_BUFFER_SIZE;
        }

        grain->progress += 1.0f;
        if (grain->progress >= grain->length) {
            grain->active = false;
        }
    }

    *out_l = mixed_l;
    *out_r = mixed_r;
}