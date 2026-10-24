#include "fx.h"
#include <math.h>

void fx_state_init(FxState *state) {
    state->lp_l = 0.0f;
    state->lp_r = 0.0f;
    state->hp_l = 0.0f;
    state->hp_r = 0.0f;
    state->ds_counter = 0.0f;
    state->held_sample_l = 0.0f;
    state->held_sample_r = 0.0f;
}

void fx_process(FxState *state, const FxParams *params, float *out_l, float *out_r) {
    float raw_l = tanhf(*out_l * 2.0f) * 0.95f * params->gain;
    float raw_r = tanhf(*out_r * 2.0f) * 0.95f * params->gain;

    if (params->downsample > 1.05f) {
        state->ds_counter += 1.0f;
        if (state->ds_counter >= params->downsample) {
            state->ds_counter = 0.0f;
            state->held_sample_l = raw_l;
            state->held_sample_r = raw_r;
        }
        raw_l = state->held_sample_l;
        raw_r = state->held_sample_r;
    }

    if (params->crush_bits < 15.9f) {
        float levels = powf(2.0f, params->crush_bits);
        raw_l = roundf(raw_l * levels) / levels;
        raw_r = roundf(raw_r * levels) / levels;
    }

    state->lp_l += 0.025f * (raw_l - state->lp_l);
    state->lp_r += 0.025f * (raw_r - state->lp_r);

    state->hp_l += 0.50f * (raw_l - state->hp_l);
    state->hp_r += 0.50f * (raw_r - state->hp_r);
    float high_l = raw_l - state->hp_l;
    float high_r = raw_r - state->hp_r;

    float bass_gain = params->tone * 1.5f;
    float air_gain = params->tone * 1.7f;
    float mid_cut = 1.0f - (params->tone * 0.22f);

    float sculpted_l = (raw_l * mid_cut) + (state->lp_l * bass_gain) + (high_l * air_gain);
    float sculpted_r = (raw_r * mid_cut) + (state->lp_r * bass_gain) + (high_r * air_gain);

    *out_l = tanhf(sculpted_l);
    *out_r = tanhf(sculpted_r);
}
