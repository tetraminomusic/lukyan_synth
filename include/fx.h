#ifndef FX_H
#define FX_H

#define CHORUS_BUFFER_SIZE 96000
#define NUM_COMB 4
#define NUM_APF 2

typedef struct {
    float crush_bits;
    float downsample;
    float gain;
    float cutoff_hz;
    float resonance;
    
    float phaser_mix;
    float phaser_rate_hz;
    float phaser_depth;
    float phaser_feedback;

    float chorus_mix;
    float chorus_rate_hz;
    float chorus_depth_ms;

    float reverb_mix;
    float reverb_size;
    float reverb_damp;
} FxParams;

typedef struct {
    float ds_counter;
    float held_sample_l;
    float held_sample_r;

    float ic1eq_l, ic2eq_l;
    float ic1eq_r, ic2eq_r;

    float phaser_lfo_phase;
    float phaser_x_l[4], phaser_y_l[4];
    float phaser_x_r[4], phaser_y_r[4];

    float chorus_buf_l[CHORUS_BUFFER_SIZE];
    float chorus_buf_r[CHORUS_BUFFER_SIZE];
    int write_pos;
    float chorus_lfo_phase;

    float comb_buf_l[NUM_COMB][2000];
    float comb_buf_r[NUM_COMB][2000];
    int comb_pos_l[NUM_COMB];
    int comb_pos_r[NUM_COMB];
    float comb_filter_l[NUM_COMB];
    float comb_filter_r[NUM_COMB];

    float apf_buf_l[NUM_APF][600];
    float apf_buf_r[NUM_APF][600];
    int apf_pos_l[NUM_APF];
    int apf_pos_r[NUM_APF];
} FxState;

void fx_state_init(FxState *state);
void fx_process(FxState *state, const FxParams *params, float *out_l, float *out_r, float sample_rate);

#endif