#include "adsr.h"

void adsr_voice_init(AdsrVoice *voice) {
    voice->state = ADSR_IDLE;
    voice->value = 0.0f;
}

void adsr_voice_gate_on(AdsrVoice *voice) {
    voice->state = ADSR_ATTACK;
}

void adsr_voice_gate_off(AdsrVoice *voice) {
    if (voice->state != ADSR_IDLE) {
        voice->state = ADSR_RELEASE;
    }
}

void adsr_voice_process(AdsrVoice *voice, const AdsrParams *params, float sr) {
    if (voice->state == ADSR_IDLE) return;

    const float attack_step = 1.0f / (sr * (params->attack_ms / 1000.0f));
    const float decay_step = (1.0f - params->sustain) / (sr * (params->decay_ms / 1000.0f));
    const float release_step = 1.0f / (sr * (params->release_ms / 1000.0f));

    switch (voice->state) {
        case ADSR_ATTACK:
            voice->value += attack_step;
            if (voice->value >= 1.0f) {
                voice->value = 1.0f;
                voice->state = ADSR_DECAY;
            }
            break;

        case ADSR_DECAY:
            voice->value -= decay_step;
            if (voice->value <= params->sustain) {
                voice->value = params->sustain;
                voice->state = ADSR_SUSTAIN;
            }
            break;

        case ADSR_SUSTAIN:
            voice->value = params->sustain;
            break;

        case ADSR_RELEASE:
            voice->value -= release_step;
            if (voice->value <= 0.0001f) {
                voice->value = 0.0f;
                voice->state = ADSR_IDLE;
            }
            break;

        default:
            break;
    }
}
