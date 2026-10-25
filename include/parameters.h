#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <clap/clap.h>
#include "granular.h"
#include "gui.h"

typedef enum {
    PARAM_GRAIN_SIZE = 0,
    PARAM_DENSITY,
    PARAM_SPRAY,
    PARAM_ATTACK,
    PARAM_DECAY,
    PARAM_SUSTAIN,
    PARAM_RELEASE,
    PARAM_GAIN,
    PARAM_CRUSH,
    PARAM_DOWNSAMPLE,
    PARAM_CUTOFF,
    PARAM_RESONANCE,
    PARAM_PHASER_MIX,
    PARAM_PHASER_RATE,
    PARAM_PHASER_DEPTH,
    PARAM_PHASER_FEEDBACK,
    PARAM_CHORUS_MIX,
    PARAM_CHORUS_RATE,
    PARAM_CHORUS_DEPTH,
    PARAM_REVERB_MIX,
    PARAM_REVERB_SIZE,
    PARAM_REVERB_DAMP,
    PARAM_OSC1_GAIN,
    PARAM_OSC1_SEMI,
    PARAM_OSC1_MORPH,
    PARAM_OSC2_GAIN,
    PARAM_OSC2_SEMI,
    PARAM_OSC2_MORPH,
    PARAM_OSC3_GAIN,
    PARAM_OSC3_SEMI,
    PARAM_OSC3_MORPH,
    PARAM_COUNT
} ParamId;

typedef struct GranularSynth {
    clap_plugin_t plugin;
    const clap_host_t *host;
    double values[PARAM_COUNT];
    GranularEngine engine;
    GuiState gui;
} GranularSynth;

uint32_t params_get_count(void);
bool params_get_info(uint32_t index, clap_param_info_t *info);
bool params_get_value(const GranularSynth *synth, clap_id param_id, double *out_value);
bool params_value_to_text(clap_id param_id, double value, char *out_buffer, uint32_t capacity);
void params_apply_value(GranularSynth *synth, clap_id param_id, double value);
void params_init_defaults(GranularSynth *synth);
bool params_state_save(const GranularSynth *synth, const clap_ostream_t *stream);
bool params_state_load(GranularSynth *synth, const clap_istream_t *stream);

#endif