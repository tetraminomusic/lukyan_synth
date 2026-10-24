#ifndef GRANULAR_H
#define GRANULAR_H

#include <stdbool.h>
#include <stdint.h>
#include "adsr.h"
#include "fx.h"
#include "oscillator.h"

#define MAX_VOICES 16
#define MAX_GRAINS 256
#define HANN_LUT_SIZE 2048
#define NUM_OSCS 3

typedef struct {
    bool active;
    float pos[NUM_OSCS];
    float speed[NUM_OSCS];
    float length;
    float progress;
    float pan;
    float amp;
} Grain;

typedef struct {
    bool active;
    int32_t key;
    float frequency;
    float spawn_timer;
    AdsrVoice adsr;
} Voice;

typedef struct {
    double sample_rate;
    float playhead;
    uint32_t next_voice_rr;

    float grain_size_ms;
    float density;
    float spray;
    float pitch_bend_semitones;

    float osc_gain[NUM_OSCS];
    float osc_semi[NUM_OSCS];
    float osc_morph[NUM_OSCS];

    AdsrParams adsr_params;
    FxParams fx_params;
    FxState fx_state;

    float hann_lut[HANN_LUT_SIZE];
    Grain grains[MAX_GRAINS];
    Voice voices[MAX_VOICES];
} GranularEngine;

void granular_init(GranularEngine *engine, double sample_rate);
void granular_note_on(GranularEngine *engine, int32_t key, float frequency);
void granular_note_off(GranularEngine *engine, int32_t key);
void granular_reset(GranularEngine *engine);
void granular_set_grain_size(GranularEngine *engine, float size_ms);
void granular_set_density(GranularEngine *engine, float density);
void granular_set_spray(GranularEngine *engine, float spray);
void granular_set_pitch_bend(GranularEngine *engine, float semitones);

void granular_set_osc_gain(GranularEngine *engine, uint32_t osc_idx, float gain);
void granular_set_osc_semi(GranularEngine *engine, uint32_t osc_idx, float semi);
void granular_set_osc_morph(GranularEngine *engine, uint32_t osc_idx, float morph);

void granular_set_attack(GranularEngine *engine, float attack_ms);
void granular_set_decay(GranularEngine *engine, float decay_ms);
void granular_set_sustain(GranularEngine *engine, float sustain);
void granular_set_release(GranularEngine *engine, float release_ms);

void granular_render_sample(GranularEngine *engine, float *out_l, float *out_r);
void granular_render_block(GranularEngine *engine, float *out_l, float *out_r, uint32_t frames);

#endif