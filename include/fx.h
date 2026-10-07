#ifndef FX_H
#define FX_H

typedef struct {
    float crush_bits;
    float downsample;
    float tone;
    float gain;
} FxParams;

typedef struct {
    float lp_l;
    float lp_r;
    float hp_l;
    float hp_r;
    float ds_counter;
    float held_sample_l;
    float held_sample_r;
} FxState;

void fx_state_init(FxState *state);
void fx_process(FxState *state, const FxParams *params, float *out_l, float *out_r);

#endif
