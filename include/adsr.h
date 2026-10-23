#ifndef ADSR_H
#define ADSR_H

#include <stdbool.h>

typedef enum {
    ADSR_IDLE = 0,
    ADSR_ATTACK,
    ADSR_DECAY,
    ADSR_SUSTAIN,
    ADSR_RELEASE
} AdsrState;

typedef struct {
    float attack_ms;
    float decay_ms;
    float sustain;
    float release_ms;
} AdsrParams;

typedef struct {
    AdsrState state;
    float value;
} AdsrVoice;

void adsr_voice_init(AdsrVoice *voice);
void adsr_voice_gate_on(AdsrVoice *voice);
void adsr_voice_gate_off(AdsrVoice *voice);
void adsr_voice_process(AdsrVoice *voice, const AdsrParams *params, float sr);

#endif
