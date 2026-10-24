#ifndef FX_H
#define FX_H

#define CHORUS_BUFFER_SIZE 96000

typedef struct {
    float crush_bits;
    float downsample;
    float gain;
    float cutoff_hz;
    float resonance;
    float chorus_mix;
    float chorus_rate_hz;
    float chorus_depth_ms;
} FxParams;

typedef struct {
    float ds_counter;
    float held_sample_l;
    float held_sample_r;

    // State Variable Filter
    float ic1eq_l, ic2eq_l;
    float ic1eq_r, ic2eq_r;

    // Chorus
    float chorus_buf_l[CHORUS_BUFFER_SIZE];
    float chorus_buf_r[CHORUS_BUFFER_SIZE];
    int write_pos;
    float lfo_phase;
} FxState;

void fx_state_init(FxState *state);
void fx_process(FxState *state, const FxParams *params, float *out_l, float *out_r, float sample_rate);

#endif