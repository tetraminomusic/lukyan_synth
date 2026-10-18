#ifndef GRANULAR_H
#define GRANULAR_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_VOICES 16
#define MAX_GRAINS 256
#define HANN_LUT_SIZE 2048
#define SAMPLE_BUFFER_SIZE 96000

typedef enum {
    ADSR_IDLE = 0,
    ADSR_ATTACK,
    ADSR_DECAY,
    ADSR_SUSTAIN,
    ADSR_RELEASE
} AdsrState;

typedef struct {
    bool active;
    float pos;
    float speed;
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
    AdsrState adsr_state;
    float adsr_value;
} Voice;

typedef struct {
    double sample_rate;
    float playhead;
    uint32_t next_voice_rr;

    float grain_size_ms;
    float density;
    float spray;
    float gain;

    float attack_ms;
    float decay_ms;
    float sustain;
    float release_ms;

    float sample_buffer[SAMPLE_BUFFER_SIZE];
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
void granular_set_gain(GranularEngine *engine, float gain);
void granular_set_attack(GranularEngine *engine, float attack_ms);
void granular_set_decay(GranularEngine *engine, float decay_ms);
void granular_set_sustain(GranularEngine *engine, float sustain);
void granular_set_release(GranularEngine *engine, float release_ms);
void granular_render_sample(GranularEngine *engine, float *out_l, float *out_r);

#endif