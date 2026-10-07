#include "granular.h"
#include <math.h>
#include <stdlib.h>

#define PI 3.14159265358979323846
#define BASE_ROOT_FREQ 110.0f

static void init_hann_lut(GranularEngine *engine) {
    for (int i = 0; i < HANN_LUT_SIZE; ++i) {
        engine->hann_lut[i] = 0.5f * (1.0f - cosf((2.0f * (float)PI * (float)i) / (float)(HANN_LUT_SIZE - 1)));
    }
}

static void init_sample_buffers(GranularEngine *engine) {
    float sr = (engine->sample_rate > 0.0) ? (float)engine->sample_rate : 44100.0f;
    float num_cycles = roundf((float)SAMPLE_BUFFER_SIZE * BASE_ROOT_FREQ / sr);

    for (int i = 0; i < SAMPLE_BUFFER_SIZE; ++i) {
        float phase = 2.0f * (float)PI * num_cycles * ((float)i / (float)SAMPLE_BUFFER_SIZE);

        engine->sample_buffers[0][i] = sinf(phase) * 0.70f + sinf(phase * 2.0f) * 0.15f;

        float tri = 0.0f;
        for (int k = 0; k < 8; ++k) {
            float n = 2.0f * (float)k + 1.0f;
            float sign = (k % 2 == 0) ? 1.0f : -1.0f;
            tri += (sign / (n * n)) * sinf(phase * n);
        }
        engine->sample_buffers[1][i] = tri * 0.70f;

        float saw = 0.0f;
        for (int k = 1; k <= 20; ++k) {
            saw += (1.0f / (float)k) * sinf(phase * (float)k);
        }
        engine->sample_buffers[2][i] = saw * 0.45f;

        float sqr = 0.0f;
        for (int k = 0; k < 10; ++k) {
            float n = 2.0f * (float)k + 1.0f;
            sqr += (1.0f / n) * sinf(phase * n);
        }
        engine->sample_buffers[3][i] = sqr * 0.50f;
    }
}

void granular_init(GranularEngine *engine, double sample_rate) {
    engine->sample_rate = sample_rate;
    engine->playhead = 0.0f;
    engine->next_voice_rr = 0;

    engine->grain_size_ms = 65.0f;
    engine->density = 30.0f;
    engine->spray = 0.15f;
    engine->gain = 0.75f;
    engine->tone = 0.50f;

    engine->osc_gain[0] = 0.80f;
    engine->osc_semi[0] = 0.0f;
    engine->osc_morph[0] = 0.0f;

    engine->osc_gain[1] = 0.45f;
    engine->osc_semi[1] = 12.0f;
    engine->osc_morph[1] = 1.0f;

    engine->osc_gain[2] = 0.35f;
    engine->osc_semi[2] = 7.0f;
    engine->osc_morph[2] = 2.0f;

    engine->attack_ms = 20.0f;
    engine->decay_ms = 200.0f;
    engine->sustain = 0.75f;
    engine->release_ms = 350.0f;

    engine->crush_bits = 16.0f;
    engine->downsample = 1.0f;
    engine->ds_counter = 0.0f;
    engine->held_sample_l = 0.0f;
    engine->held_sample_r = 0.0f;
    engine->pitch_bend_semitones = 0.0f;

    engine->lp_l = 0.0f;
    engine->lp_r = 0.0f;
    engine->hp_l = 0.0f;
    engine->hp_r = 0.0f;

    init_hann_lut(engine);
    init_sample_buffers(engine);

    for (int i = 0; i < MAX_GRAINS; ++i) {
        engine->grains[i].active = false;
        engine->grains[i].amp = 1.0f;
        for (int o = 0; o < NUM_OSCS; ++o) {
            engine->grains[i].pos[o] = 0.0f;
            engine->grains[i].speed[o] = 1.0f;
        }
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

void granular_set_gain(GranularEngine *engine, float gain) {
    engine->gain = gain;
}

void granular_set_tone(GranularEngine *engine, float tone) {
    if (tone < 0.0f) tone = 0.0f;
    if (tone > 1.0f) tone = 1.0f;
    engine->tone = tone;
}

void granular_set_osc_gain(GranularEngine *engine, uint32_t osc_idx, float gain) {
    if (osc_idx < NUM_OSCS) engine->osc_gain[osc_idx] = gain;
}

void granular_set_osc_semi(GranularEngine *engine, uint32_t osc_idx, float semi) {
    if (osc_idx < NUM_OSCS) engine->osc_semi[osc_idx] = semi;
}

void granular_set_osc_morph(GranularEngine *engine, uint32_t osc_idx, float morph) {
    if (osc_idx < NUM_OSCS) {
        if (morph < 0.0f) morph = 0.0f;
        if (morph > 3.0f) morph = 3.0f;
        engine->osc_morph[osc_idx] = morph;
    }
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

void granular_set_crush(GranularEngine *engine, float bits) {
    engine->crush_bits = bits;
}

void granular_set_downsample(GranularEngine *engine, float factor) {
    engine->downsample = factor;
}

void granular_set_pitch_bend(GranularEngine *engine, float semitones) {
    engine->pitch_bend_semitones = semitones;
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
    engine->pitch_bend_semitones = 0.0f;
    engine->lp_l = 0.0f;
    engine->lp_r = 0.0f;
    engine->hp_l = 0.0f;
    engine->hp_r = 0.0f;
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

            float bent_freq = frequency * powf(2.0f, engine->pitch_bend_semitones / 12.0f);
            float base_spd = bent_freq / BASE_ROOT_FREQ;

            for (int o = 0; o < NUM_OSCS; ++o) {
                float osc_ratio = powf(2.0f, engine->osc_semi[o] / 12.0f);
                engine->grains[i].pos[o] = start_pos;
                engine->grains[i].speed[o] = base_spd * osc_ratio;
            }

            engine->grains[i].length = (float)engine->sample_rate * (engine->grain_size_ms / 1000.0f);
            engine->grains[i].progress = 0.0f;
            engine->grains[i].pan = (float)(rand() % 1000) / 1000.0f;
            engine->grains[i].amp = amp;
            break;
        }
    }
}

static inline float read_morphed(const float buffers[NUM_WAVEFORMS][SAMPLE_BUFFER_SIZE], float pos, float morph) {
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

    float val0 = buffers[t0][idx_a] * (1.0f - frac_p) + buffers[t0][idx_b] * frac_p;
    float val1 = buffers[t1][idx_a] * (1.0f - frac_p) + buffers[t1][idx_b] * frac_p;

    return val0 * (1.0f - frac_m) + val1 * frac_m;
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

        float audio_val = 0.0f;
        for (int o = 0; o < NUM_OSCS; ++o) {
            if (engine->osc_gain[o] > 0.001f) {
                audio_val += read_morphed(engine->sample_buffers, grain->pos[o], engine->osc_morph[o]) * engine->osc_gain[o];
            }
            grain->pos[o] += grain->speed[o];
            while (grain->pos[o] >= (float)SAMPLE_BUFFER_SIZE) grain->pos[o] -= (float)SAMPLE_BUFFER_SIZE;
        }

        float grain_amp = audio_val * env * grain->amp * 0.030f;
        mixed_l += grain_amp * (1.0f - grain->pan);
        mixed_r += grain_amp * grain->pan;

        grain->progress += 1.0f;
        if (grain->progress >= grain->length) {
            grain->active = false;
        }
    }

    float out_raw_l = tanhf(mixed_l * 1.15f) * 0.85f * engine->gain;
    float out_raw_r = tanhf(mixed_r * 1.15f) * 0.85f * engine->gain;

    if (engine->downsample > 1.05f) {
        engine->ds_counter += 1.0f;
        if (engine->ds_counter >= engine->downsample) {
            engine->ds_counter = 0.0f;
            engine->held_sample_l = out_raw_l;
            engine->held_sample_r = out_raw_r;
        }
        out_raw_l = engine->held_sample_l;
        out_raw_r = engine->held_sample_r;
    }

    if (engine->crush_bits < 15.9f) {
        float levels = powf(2.0f, engine->crush_bits);
        out_raw_l = roundf(out_raw_l * levels) / levels;
        out_raw_r = roundf(out_raw_r * levels) / levels;
    }

    engine->lp_l += 0.025f * (out_raw_l - engine->lp_l);
    engine->lp_r += 0.025f * (out_raw_r - engine->lp_r);

    engine->hp_l += 0.50f * (out_raw_l - engine->hp_l);
    engine->hp_r += 0.50f * (out_raw_r - engine->hp_r);
    float high_l = out_raw_l - engine->hp_l;
    float high_r = out_raw_r - engine->hp_r;

    float bass_gain = engine->tone * 1.5f;
    float air_gain = engine->tone * 1.7f;
    float mid_cut = 1.0f - (engine->tone * 0.22f);

    float sculpted_l = (out_raw_l * mid_cut) + (engine->lp_l * bass_gain) + (high_l * air_gain);
    float sculpted_r = (out_raw_r * mid_cut) + (engine->lp_r * bass_gain) + (high_r * air_gain);

    *out_l = tanhf(sculpted_l);
    *out_r = tanhf(sculpted_r);
}