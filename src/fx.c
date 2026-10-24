#include "fx.h"
#include <math.h>

#define PI 3.14159265358979323846

void fx_state_init(FxState *state) {
    state->ds_counter = 0.0f;
    state->held_sample_l = 0.0f;
    state->held_sample_r = 0.0f;
    state->ic1eq_l = 0.0f; state->ic2eq_l = 0.0f;
    state->ic1eq_r = 0.0f; state->ic2eq_r = 0.0f;
    
    state->write_pos = 0;
    state->lfo_phase = 0.0f;
    for (int i = 0; i < CHORUS_BUFFER_SIZE; ++i) {
        state->chorus_buf_l[i] = 0.0f;
        state->chorus_buf_r[i] = 0.0f;
    }
}

static float read_delay(const float *buffer, float read_pos) {
    int idx1 = (int)read_pos;
    int idx2 = (idx1 + 1) % CHORUS_BUFFER_SIZE;
    float frac = read_pos - (float)idx1;
    return buffer[idx1] * (1.0f - frac) + buffer[idx2] * frac;
}

void fx_process(FxState *state, const FxParams *params, float *out_l, float *out_r, float sample_rate) {
    float sr = (sample_rate > 0.0f) ? sample_rate : 44100.0f;

    float raw_l = tanhf(*out_l * 2.0f) * 0.95f * params->gain;
    float raw_r = tanhf(*out_r * 2.0f) * 0.95f * params->gain;

    if (params->downsample > 1.05f) {
        state->ds_counter += 1.0f;
        if (state->ds_counter >= params->downsample) {
            state->ds_counter -= params->downsample;
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

    float g = tanf(PI * params->cutoff_hz / sr);
    float k = 1.0f / params->resonance;
    float a1 = 1.0f / (1.0f + g * (g + k));
    float a2 = g * a1;
    float a3 = g * a2;

    float v3_l = raw_l - state->ic2eq_l;
    float v1_l = a1 * state->ic1eq_l + a2 * v3_l;
    float v2_l = state->ic2eq_l + a2 * state->ic1eq_l + a3 * v3_l;
    state->ic1eq_l = 2.0f * v1_l - state->ic1eq_l;
    state->ic2eq_l = 2.0f * v2_l - state->ic2eq_l;

    float v3_r = raw_r - state->ic2eq_r;
    float v1_r = a1 * state->ic1eq_r + a2 * v3_r;
    float v2_r = state->ic2eq_r + a2 * state->ic1eq_r + a3 * v3_r;
    state->ic1eq_r = 2.0f * v1_r - state->ic1eq_r;
    state->ic2eq_r = 2.0f * v2_r - state->ic2eq_r;

    float filtered_l = v2_l;
    float filtered_r = v2_r;

    state->chorus_buf_l[state->write_pos] = filtered_l;
    state->chorus_buf_r[state->write_pos] = filtered_r;

    state->lfo_phase += (2.0f * (float)PI * params->chorus_rate_hz) / sr;
    if (state->lfo_phase > 2.0f * PI) state->lfo_phase -= 2.0f * PI;

    float lfo_val_l = sinf(state->lfo_phase);
    float lfo_val_r = sinf(state->lfo_phase + PI * 0.5f);

    float base_delay_samples = 0.015f * sr; 
    float mod_samples = (params->chorus_depth_ms / 1000.0f) * sr;

    float delay_l = base_delay_samples + lfo_val_l * mod_samples;
    float delay_r = base_delay_samples + lfo_val_r * mod_samples;

    float read_pos_l = (float)state->write_pos - delay_l;
    while (read_pos_l < 0.0f) read_pos_l += CHORUS_BUFFER_SIZE;
    
    float read_pos_r = (float)state->write_pos - delay_r;
    while (read_pos_r < 0.0f) read_pos_r += CHORUS_BUFFER_SIZE;

    float chorus_l = read_delay(state->chorus_buf_l, read_pos_l);
    float chorus_r = read_delay(state->chorus_buf_r, read_pos_r);

    state->write_pos = (state->write_pos + 1) % CHORUS_BUFFER_SIZE;

    *out_l = filtered_l * (1.0f - params->chorus_mix) + chorus_l * params->chorus_mix;
    *out_r = filtered_r * (1.0f - params->chorus_mix) + chorus_r * params->chorus_mix;
}