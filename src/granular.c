#include "granular.h"
#include <math.h>
#include <stdlib.h>

#define PI 3.14159265358979323846
#define BASE_ROOT_FREQ 220.0f

static void init_hann_lut(GranularEngine *engine) {
    for (int i = 0; i < HANN_LUT_SIZE; ++i) {
        engine->hann_lut[i] = 0.5f * (1.0f - cosf((2.0f * (float)PI * (float)i) / (float)(HANN_LUT_SIZE - 1)));
    }
}

static void init_sample_buffer(GranularEngine *engine) {
    float sr = (engine->sample_rate > 0.0) ? (float)engine->sample_rate : 44100.0f;

    float num_cycles = roundf((float)SAMPLE_BUFFER_SIZE * BASE_ROOT_FREQ / sr);

    for (int i = 0; i < SAMPLE_BUFFER_SIZE; ++i) {
        float phase = 2.0f * (float)PI * num_cycles * ((float)i / (float)SAMPLE_BUFFER_SIZE);
        
        float wave = sinf(phase) * 0.55f
                   + sinf(phase * 2.0f) * 0.30f
                   + sinf(phase * 4.0f) * 0.12f
                   + sinf(phase * 8.0f) * 0.03f;

        engine->sample_buffer[i] = wave;
    }
}

void granular_init(GranularEngine *engine, double sample_rate) {
    engine->sample_rate = sample_rate;
    engine->playhead = 0.0f;
    engine->next_voice_rr = 0;

    engine->grain_size_ms = 65.0f;
    engine->density = 30.0f;
    engine->spray = 0.15f;

    engine->attack_ms = 20.0f;
    engine->decay_ms = 200.0f;
    engine->sustain = 0.75f;
    engine->release_ms = 350.0f;

    init_hann_lut(engine);
    init_sample_buffer(engine);

    for (int i = 0; i < MAX_GRAINS; ++i) {
        engine->grains[i].active = false;
        engine->grains[i].amp = 1.0f;
    }

    for (int v = 0; v < MAX_VOICES; ++v) {
        engine->voices[v].active = false;
        engine->voices[v].key = -1;
        engine->voices[v].frequency = 440.0f;
        engine->voices[v].spawn_timer = 0.0f;
        engine->voices[v].adsr_state = ADSR_IDLE;
        engine->voices[v].adsr_value = 0.0f;
    }
}

void granular_set_grain_size(GranularEngine *engine, float size_ms) {
    engine->grain_size_ms = size_ms;
}

void granular_set_density(GranularEngine *engine, float density) {
    engine->density = density;
}

void granular_set_spray(GranularEngine *engine, float spray) {
    engine->spray = spray;
}

void granular_set_attack(GranularEngine *engine, float attack_ms) {
    engine->attack_ms = attack_ms;
}

void granular_set_decay(GranularEngine *engine, float decay_ms) {
    engine->decay_ms = decay_ms;
}

void granular_set_sustain(GranularEngine *engine, float sustain) {
    engine->sustain = sustain;
}

void granular_set_release(GranularEngine *engine, float release_ms) {
    engine->release_ms = release_ms;
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
        target_idx = engine->next_voice_rr;
        engine->next_voice_rr = (engine->next_voice_rr + 1) % MAX_VOICES;
    }

    engine->voices[target_idx].active = true;
    engine->voices[target_idx].key = key;
    engine->voices[target_idx].frequency = frequency;
    engine->voices[target_idx].spawn_timer = 0.0f;
    engine->voices[target_idx].adsr_state = ADSR_ATTACK;
}

void granular_note_off(GranularEngine *engine, int32_t key) {
    for (int v = 0; v < MAX_VOICES; ++v) {
        if (engine->voices[v].active && engine->voices[v].key == key && engine->voices[v].adsr_state != ADSR_RELEASE) {
            engine->voices[v].adsr_state = ADSR_RELEASE;
        }
    }
}

void granular_reset(GranularEngine *engine) {
    for (int v = 0; v < MAX_VOICES; ++v) {
        engine->voices[v].active = false;
        engine->voices[v].key = -1;
        engine->voices[v].spawn_timer = 0.0f;
        engine->voices[v].adsr_state = ADSR_IDLE;
        engine->voices[v].adsr_value = 0.0f;
    }
    for (int i = 0; i < MAX_GRAINS; ++i) {
        engine->grains[i].active = false;
    }
    engine->playhead = 0.0f;
    engine->next_voice_rr = 0;
}

static void spawn_grain(GranularEngine *engine, float frequency, float amp) {
    for (int i = 0; i < MAX_GRAINS; ++i) {
        if (!engine->grains[i].active) {
            engine->grains[i].active = true;

            float max_jitter = 6000.0f * engine->spray;
            float jitter = ((float)(rand() % 2000) / 1000.0f - 1.0f) * max_jitter;
            float start_pos = engine->playhead + jitter;
            while (start_pos < 0.0f) start_pos += (float)SAMPLE_BUFFER_SIZE;
            while (start_pos >= (float)SAMPLE_BUFFER_SIZE) start_pos -= (float)SAMPLE_BUFFER_SIZE;

            engine->grains[i].pos = start_pos;
            engine->grains[i].speed = frequency / BASE_ROOT_FREQ;
            engine->grains[i].length = (float)engine->sample_rate * (engine->grain_size_ms / 1000.0f);
            engine->grains[i].progress = 0.0f;
            engine->grains[i].pan = (float)(rand() % 1000) / 1000.0f;
            engine->grains[i].amp = amp;
            break;
        }
    }
}

void granular_render_sample(GranularEngine *engine, float *out_l, float *out_r) {
    const float sr = (engine->sample_rate > 0.0) ? (float)engine->sample_rate : 44100.0f;
    const float spawn_interval = (engine->density > 0.0f) ? (sr / engine->density) : 1000.0f;

    const float attack_step = 1.0f / (sr * (engine->attack_ms / 1000.0f));
    const float decay_step = (1.0f - engine->sustain) / (sr * (engine->decay_ms / 1000.0f));
    const float release_step = 1.0f / (sr * (engine->release_ms / 1000.0f));

    for (int v = 0; v < MAX_VOICES; ++v) {
        if (!engine->voices[v].active) continue;

        Voice *voice = &engine->voices[v];

        switch (voice->adsr_state) {
            case ADSR_ATTACK:
                voice->adsr_value += attack_step;
                if (voice->adsr_value >= 1.0f) {
                    voice->adsr_value = 1.0f;
                    voice->adsr_state = ADSR_DECAY;
                }
                break;

            case ADSR_DECAY:
                voice->adsr_value -= decay_step;
                if (voice->adsr_value <= engine->sustain) {
                    voice->adsr_value = engine->sustain;
                    voice->adsr_state = ADSR_SUSTAIN;
                }
                break;

            case ADSR_SUSTAIN:
                voice->adsr_value = engine->sustain;
                break;

            case ADSR_RELEASE:
                voice->adsr_value -= release_step;
                if (voice->adsr_value <= 0.0001f) {
                    voice->adsr_value = 0.0f;
                    voice->adsr_state = ADSR_IDLE;
                    voice->active = false;
                    voice->key = -1;
                }
                break;

            default:
                break;
        }

        if (voice->active && voice->adsr_value > 0.0f) {
            voice->spawn_timer += 1.0f;
            if (voice->spawn_timer >= spawn_interval) {
                voice->spawn_timer = 0.0f;
                spawn_grain(engine, voice->frequency, voice->adsr_value);
            }
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

        float grain_amp = audio_val * env * grain->amp * 0.045f;
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

    *out_l = tanhf(mixed_l * 1.15f) * 0.85f;
    *out_r = tanhf(mixed_r * 1.15f) * 0.85f;
}