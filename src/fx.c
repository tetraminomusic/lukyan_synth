#include "fx.h"
#include <math.h>

#define PI 3.14159265358979323846

void fx_state_init(FxState *state) {
    state->ds_counter = 0.0f;
    state->held_sample_l = 0.0f;
    state->held_sample_r = 0.0f;
    
    state->ic1eq_l = 0.0f; state->ic2eq_l = 0.0f;
    state->ic1eq_r = 0.0f; state->ic2eq_r = 0.0f;
    
    state->phaser_lfo_phase = 0.0f;
    for (int i = 0; i < 4; ++i) {
        state->phaser_x_l[i] = 0.0f; state->phaser_y_l[i] = 0.0f;
        state->phaser_x_r[i] = 0.0f; state->phaser_y_r[i] = 0.0f;
    }

    state->write_pos = 0;
    state->chorus_lfo_phase = 0.0f;
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

    state->phaser_lfo_phase += (2.0f * (float)PI * params->phaser_rate_hz) / sr;
    if (state->phaser_lfo_phase > 2.0f * PI) state->phaser_lfo_phase -= 2.0f * PI;

    float ph_lfo_l = (sinf(state->phaser_lfo_phase) + 1.0f) * 0.5f;
    float ph_lfo_r = (sinf(state->phaser_lfo_phase + PI * 0.5f) + 1.0f) * 0.5f;

    float sweep_l = 400.0f + (4000.0f - 400.0f) * ph_lfo_l * params->phaser_depth;
    float sweep_r = 400.0f + (4000.0f - 400.0f) * ph_lfo_r * params->phaser_depth;

    float apf_a_l = (tanf(PI * sweep_l / sr) - 1.0f) / (tanf(PI * sweep_l / sr) + 1.0f);
    float apf_a_r = (tanf(PI * sweep_r / sr) - 1.0f) / (tanf(PI * sweep_r / sr) + 1.0f);

    float p_in_l = filtered_l + state->phaser_y_l[3] * params->phaser_feedback;
    for (int i = 0; i < 4; ++i) {
        float out = apf_a_l * p_in_l + state->phaser_x_l[i] - apf_a_l * state->phaser_y_l[i];
        state->phaser_x_l[i] = p_in_l;
        state->phaser_y_l[i] = out;
        p_in_l = out;
    }
    float phaser_out_l = filtered_l * (1.0f - params->phaser_mix) + p_in_l * params->phaser_mix;

    float p_in_r = filtered_r + state->phaser_y_r[3] * params->phaser_feedback;
    for (int i = 0; i < 4; ++i) {
        float out = apf_a_r * p_in_r + state->phaser_x_r[i] - apf_a_r * state->phaser_y_r[i];
        state->phaser_x_r[i] = p_in_r;
        state->phaser_y_r[i] = out;
        p_in_r = out;
    }
    float phaser_out_r = filtered_r * (1.0f - params->phaser_mix) + p_in_r * params->phaser_mix;

    state->chorus_buf_l[state->write_pos] = phaser_out_l;
    state->chorus_buf_r[state->write_pos] = phaser_out_r;

    state->chorus_lfo_phase += (2.0f * (float)PI * params->chorus_rate_hz) / sr;
    if (state->chorus_lfo_phase > 2.0f * PI) state->chorus_lfo_phase -= 2.0f * PI;

    float ch_lfo_l = sinf(state->chorus_lfo_phase);
    float ch_lfo_r = sinf(state->chorus_lfo_phase + PI * 0.5f);

    float base_delay_samples = 0.015f * sr; 
    float mod_samples = (params->chorus_depth_ms / 1000.0f) * sr;

    float delay_l = base_delay_samples + ch_lfo_l * mod_samples;
    float delay_r = base_delay_samples + ch_lfo_r * mod_samples;

    float read_pos_l = (float)state->write_pos - delay_l;
    while (read_pos_l < 0.0f) read_pos_l += CHORUS_BUFFER_SIZE;
    
    float read_pos_r = (float)state->write_pos - delay_r;
    while (read_pos_r < 0.0f) read_pos_r += CHORUS_BUFFER_SIZE;

    float chorus_l = read_delay(state->chorus_buf_l, read_pos_l);
    float chorus_r = read_delay(state->chorus_buf_r, read_pos_r);

    state->write_pos = (state->write_pos + 1) % CHORUS_BUFFER_SIZE;

    *out_l = phaser_out_l * (1.0f - params->chorus_mix) + chorus_l * params->chorus_mix;
    *out_r = phaser_out_r * (1.0f - params->chorus_mix) + chorus_r * params->chorus_mix;
}