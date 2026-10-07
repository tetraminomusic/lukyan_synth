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

void granular_init(GranularEngine *engine, double sample_rate) {
    engine->sample_rate = sample_rate;
    engine->playhead = 0.0f;
    engine->next_voice_rr = 0;

    engine->grain_size_ms = 65.0f;
    engine->density = 30.0f;
    engine->spray = 0.15f;
    engine->pitch_bend_semitones = 0.0f;

    engine->osc_gain[0] = 0.80f;
    engine->osc_semi[0] = 0.0f;
    engine->osc_morph[0] = 0.0f;

    engine->osc_gain[1] = 0.45f;
    engine->osc_semi[1] = 12.0f;
    engine->osc_morph[1] = 1.0f;

    engine->osc_gain[2] = 0.35f;
    engine->osc_semi[2] = 7.0f;
    engine->osc_morph[2] = 2.0f;

    engine->adsr_params.attack_ms = 20.0f;
    engine->adsr_params.decay_ms = 200.0f;
    engine->adsr_params.sustain = 0.75f;
    engine->adsr_params.release_ms = 350.0f;

    engine->fx_params.gain = 0.75f;
    engine->fx_params.cutoff_hz = 20000.0f;
    engine->fx_params.resonance = 0.707f;
    engine->fx_params.phaser_mix = 0.0f;
    engine->fx_params.phaser_rate_hz = 0.5f;
    engine->fx_params.phaser_depth = 0.8f;
    engine->fx_params.phaser_feedback = 0.5f;
    engine->fx_params.chorus_mix = 0.0f;
    engine->fx_params.chorus_rate_hz = 1.2f;
    engine->fx_params.chorus_depth_ms = 5.0f;
    engine->fx_params.reverb_mix = 0.0f;
    engine->fx_params.reverb_size = 0.75f;
    engine->fx_params.reverb_damp = 0.25f;
    engine->fx_params.crush_bits = 16.0f;
    engine->fx_params.downsample = 1.0f;

    fx_state_init(&engine->fx_state);
    init_hann_lut(engine);
    oscillator_init_buffers(sample_rate);

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
        adsr_voice_init(&engine->voices[v].adsr);
    }
}

void granular_set_grain_size(GranularEngine *engine, float size_ms) { engine->grain_size_ms = size_ms; }
void granular_set_density(GranularEngine *engine, float density) { engine->density = density; }
void granular_set_spray(GranularEngine *engine, float spray) { engine->spray = spray; }
void granular_set_pitch_bend(GranularEngine *engine, float semitones) { engine->pitch_bend_semitones = semitones; }

void granular_set_osc_gain(GranularEngine *engine, uint32_t osc_idx, float gain) { if (osc_idx < NUM_OSCS) engine->osc_gain[osc_idx] = gain; }
void granular_set_osc_semi(GranularEngine *engine, uint32_t osc_idx, float semi) { if (osc_idx < NUM_OSCS) engine->osc_semi[osc_idx] = semi; }
void granular_set_osc_morph(GranularEngine *engine, uint32_t osc_idx, float morph) {
    if (osc_idx < NUM_OSCS) engine->osc_morph[osc_idx] = (morph < 0.0f) ? 0.0f : ((morph > 3.0f) ? 3.0f : morph);
}

void granular_set_attack(GranularEngine *engine, float attack_ms) { engine->adsr_params.attack_ms = attack_ms; }
void granular_set_decay(GranularEngine *engine, float decay_ms) { engine->adsr_params.decay_ms = decay_ms; }
void granular_set_sustain(GranularEngine *engine, float sustain) { engine->adsr_params.sustain = sustain; }
void granular_set_release(GranularEngine *engine, float release_ms) { engine->adsr_params.release_ms = release_ms; }

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
    adsr_voice_gate_on(&engine->voices[target_idx].adsr);
}

void granular_note_off(GranularEngine *engine, int32_t key) {
    for (int v = 0; v < MAX_VOICES; ++v) {
        if (engine->voices[v].active && engine->voices[v].key == key) {
            adsr_voice_gate_off(&engine->voices[v].adsr);
        }
    }
}

void granular_reset(GranularEngine *engine) {
    for (int v = 0; v < MAX_VOICES; ++v) {
        engine->voices[v].active = false;
        engine->voices[v].key = -1;
        engine->voices[v].spawn_timer = 0.0f;
        adsr_voice_init(&engine->voices[v].adsr);
    }
    for (int i = 0; i < MAX_GRAINS; ++i) {
        engine->grains[i].active = false;
    }
    engine->playhead = 0.0f;
    engine->next_voice_rr = 0;
    engine->pitch_bend_semitones = 0.0f;
    fx_state_init(&engine->fx_state);
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

void granular_render_sample(GranularEngine *engine, float *out_l, float *out_r) {
    const float sr = (engine->sample_rate > 0.0) ? (float)engine->sample_rate : 44100.0f;
    const float spawn_interval = (engine->density > 0.0f) ? (sr / engine->density) : 1000.0f;

    for (int v = 0; v < MAX_VOICES; ++v) {
        if (!engine->voices[v].active) continue;

        Voice *voice = &engine->voices[v];
        adsr_voice_process(&voice->adsr, &engine->adsr_params, sr);

        if (voice->adsr.state == ADSR_IDLE) {
            voice->active = false;
            voice->key = -1;
            continue;
        }

        if (voice->adsr.value > 0.0f) {
            voice->spawn_timer += 1.0f;
            if (voice->spawn_timer >= spawn_interval) {
                voice->spawn_timer = 0.0f;
                spawn_grain(engine, voice->frequency, voice->adsr.value);
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
                audio_val += oscillator_read_morphed(grain->pos[o], engine->osc_morph[o]) * engine->osc_gain[o];
            }
            grain->pos[o] += grain->speed[o];
            while (grain->pos[o] >= (float)SAMPLE_BUFFER_SIZE) grain->pos[o] -= (float)SAMPLE_BUFFER_SIZE;
        }

        float grain_amp = audio_val * env * grain->amp * 0.150f;
        mixed_l += grain_amp * (1.0f - grain->pan);
        mixed_r += grain_amp * grain->pan;

        grain->progress += 1.0f;
        if (grain->progress >= grain->length) {
            grain->active = false;
        }
    }

    fx_process(&engine->fx_state, &engine->fx_params, &mixed_l, &mixed_r, sr);

    *out_l = mixed_l;
    *out_r = mixed_r;
}

void granular_render_block(GranularEngine *engine, float *out_l, float *out_r, uint32_t frames) {
    for (uint32_t i = 0; i < frames; ++i) {
        float sample_l = 0.0f;
        float sample_r = 0.0f;

        granular_render_sample(engine, &sample_l, &sample_r);

        if (out_l) out_l[i] = sample_l;
        if (out_r) out_r[i] = sample_r;
    }
}