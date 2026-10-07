#include "parameters.h"
#include <stdio.h>
#include <string.h>

typedef void (*FormatFunc)(double val, char *buf, uint32_t cap);
typedef void (*ApplyFunc)(GranularEngine *engine, double val);

typedef struct {
    clap_id id;
    const char *name;
    double min_val;
    double max_val;
    double def_val;
    uint32_t flags;
    FormatFunc format;
    ApplyFunc apply;
} ParamDef;

static void fmt_ms(double val, char *buf, uint32_t cap) { snprintf(buf, cap, "%.1f ms", val); }
static void fmt_gr_s(double val, char *buf, uint32_t cap) { snprintf(buf, cap, "%.1f gr/s", val); }
static void fmt_norm(double val, char *buf, uint32_t cap) { snprintf(buf, cap, "%.2f", val); }
static void fmt_pct(double val, char *buf, uint32_t cap) { snprintf(buf, cap, "%.0f %%", val * 100.0); }
static void fmt_semi(double val, char *buf, uint32_t cap) { snprintf(buf, cap, "%+.0f st", val); }
static void fmt_bits(double val, char *buf, uint32_t cap) { snprintf(buf, cap, "%.1f bit", val); }
static void fmt_rate(double val, char *buf, uint32_t cap) { snprintf(buf, cap, "%.0fx", val); }

static void fmt_morph(double val, char *buf, uint32_t cap) {
    if (val < 0.1) snprintf(buf, cap, "Sine");
    else if (val < 0.9) snprintf(buf, cap, "Sine->Tri");
    else if (val < 1.1) snprintf(buf, cap, "Triangle");
    else if (val < 1.9) snprintf(buf, cap, "Tri->Saw");
    else if (val < 2.1) snprintf(buf, cap, "Saw");
    else if (val < 2.9) snprintf(buf, cap, "Saw->Sqr");
    else snprintf(buf, cap, "Square");
}

static void app_grain_size(GranularEngine *e, double v) { granular_set_grain_size(e, (float)v); }
static void app_density(GranularEngine *e, double v) { granular_set_density(e, (float)v); }
static void app_spray(GranularEngine *e, double v) { granular_set_spray(e, (float)v); }
static void app_attack(GranularEngine *e, double v) { granular_set_attack(e, (float)v); }
static void app_decay(GranularEngine *e, double v) { granular_set_decay(e, (float)v); }
static void app_sustain(GranularEngine *e, double v) { granular_set_sustain(e, (float)v); }
static void app_release(GranularEngine *e, double v) { granular_set_release(e, (float)v); }
static void app_gain(GranularEngine *e, double v) { granular_set_gain(e, (float)v); }
static void app_crush(GranularEngine *e, double v) { granular_set_crush(e, (float)v); }
static void app_downsample(GranularEngine *e, double v) { granular_set_downsample(e, (float)v); }
static void app_tone(GranularEngine *e, double v) { granular_set_tone(e, (float)v); }

static void app_osc1_gain(GranularEngine *e, double v) { granular_set_osc_gain(e, 0, (float)v); }
static void app_osc1_semi(GranularEngine *e, double v) { granular_set_osc_semi(e, 0, (float)v); }
static void app_osc1_morph(GranularEngine *e, double v) { granular_set_osc_morph(e, 0, (float)v); }

static void app_osc2_gain(GranularEngine *e, double v) { granular_set_osc_gain(e, 1, (float)v); }
static void app_osc2_semi(GranularEngine *e, double v) { granular_set_osc_semi(e, 1, (float)v); }
static void app_osc2_morph(GranularEngine *e, double v) { granular_set_osc_morph(e, 1, (float)v); }

static void app_osc3_gain(GranularEngine *e, double v) { granular_set_osc_gain(e, 2, (float)v); }
static void app_osc3_semi(GranularEngine *e, double v) { granular_set_osc_semi(e, 2, (float)v); }
static void app_osc3_morph(GranularEngine *e, double v) { granular_set_osc_morph(e, 2, (float)v); }

static const ParamDef DEFS[PARAM_COUNT] = {
    { PARAM_GRAIN_SIZE, "Grain Size", 10.0, 200.0, 65.0, CLAP_PARAM_IS_AUTOMATABLE, fmt_ms, app_grain_size },
    { PARAM_DENSITY, "Density", 5.0, 100.0, 30.0, CLAP_PARAM_IS_AUTOMATABLE, fmt_gr_s, app_density },
    { PARAM_SPRAY, "Spray", 0.0, 1.0, 0.15, CLAP_PARAM_IS_AUTOMATABLE, fmt_norm, app_spray },
    { PARAM_ATTACK, "Attack", 1.0, 2000.0, 20.0, CLAP_PARAM_IS_AUTOMATABLE, fmt_ms, app_attack },
    { PARAM_DECAY, "Decay", 10.0, 2000.0, 200.0, CLAP_PARAM_IS_AUTOMATABLE, fmt_ms, app_decay },
    { PARAM_SUSTAIN, "Sustain", 0.0, 1.0, 0.75, CLAP_PARAM_IS_AUTOMATABLE, fmt_norm, app_sustain },
    { PARAM_RELEASE, "Release", 10.0, 3000.0, 350.0, CLAP_PARAM_IS_AUTOMATABLE, fmt_ms, app_release },
    { PARAM_GAIN, "Master Gain", 0.0, 1.0, 0.75, CLAP_PARAM_IS_AUTOMATABLE, fmt_pct, app_gain },
    { PARAM_CRUSH, "Lo-Fi Bits", 4.0, 16.0, 16.0, CLAP_PARAM_IS_AUTOMATABLE, fmt_bits, app_crush },
    { PARAM_DOWNSAMPLE, "Lo-Fi Rate", 1.0, 24.0, 1.0, CLAP_PARAM_IS_AUTOMATABLE, fmt_rate, app_downsample },
    { PARAM_TONE, "Tone (Hi-Fi)", 0.0, 1.0, 0.50, CLAP_PARAM_IS_AUTOMATABLE, fmt_pct, app_tone },
    { PARAM_OSC1_GAIN, "Osc 1 Gain", 0.0, 1.0, 0.80, CLAP_PARAM_IS_AUTOMATABLE, fmt_pct, app_osc1_gain },
    { PARAM_OSC1_SEMI, "Osc 1 Semi", -24.0, 24.0, 0.0, CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_IS_STEPPED, fmt_semi, app_osc1_semi },
    { PARAM_OSC1_MORPH, "Osc 1 Morph", 0.0, 3.0, 0.0, CLAP_PARAM_IS_AUTOMATABLE, fmt_morph, app_osc1_morph },
    { PARAM_OSC2_GAIN, "Osc 2 Gain", 0.0, 1.0, 0.45, CLAP_PARAM_IS_AUTOMATABLE, fmt_pct, app_osc2_gain },
    { PARAM_OSC2_SEMI, "Osc 2 Semi", -24.0, 24.0, 12.0, CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_IS_STEPPED, fmt_semi, app_osc2_semi },
    { PARAM_OSC2_MORPH, "Osc 2 Morph", 0.0, 3.0, 1.0, CLAP_PARAM_IS_AUTOMATABLE, fmt_morph, app_osc2_morph },
    { PARAM_OSC3_GAIN, "Osc 3 Gain", 0.0, 1.0, 0.35, CLAP_PARAM_IS_AUTOMATABLE, fmt_pct, app_osc3_gain },
    { PARAM_OSC3_SEMI, "Osc 3 Semi", -24.0, 24.0, 7.0, CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_IS_STEPPED, fmt_semi, app_osc3_semi },
    { PARAM_OSC3_MORPH, "Osc 3 Morph", 0.0, 3.0, 2.0, CLAP_PARAM_IS_AUTOMATABLE, fmt_morph, app_osc3_morph },
};

uint32_t params_get_count(void) {
    return PARAM_COUNT;
}

bool params_get_info(uint32_t index, clap_param_info_t *info) {
    if (!info || index >= PARAM_COUNT) return false;

    memset(info, 0, sizeof(*info));
    info->id = DEFS[index].id;
    info->flags = DEFS[index].flags;
    strncpy(info->name, DEFS[index].name, sizeof(info->name));
    info->min_value = DEFS[index].min_val;
    info->max_value = DEFS[index].max_val;
    info->default_value = DEFS[index].def_val;

    return true;
}

bool params_get_value(const GranularSynth *synth, clap_id param_id, double *out_value) {
    if (!synth || !out_value || param_id >= PARAM_COUNT) return false;
    *out_value = synth->values[param_id];
    return true;
}

bool params_value_to_text(clap_id param_id, double value, char *out_buffer, uint32_t capacity) {
    if (!out_buffer || capacity == 0 || param_id >= PARAM_COUNT) return false;
    DEFS[param_id].format(value, out_buffer, capacity);
    return true;
}

void params_apply_value(GranularSynth *synth, clap_id param_id, double value) {
    if (!synth || param_id >= PARAM_COUNT) return;
    synth->values[param_id] = value;
    DEFS[param_id].apply(&synth->engine, value);
}

void params_init_defaults(GranularSynth *synth) {
    for (int i = 0; i < PARAM_COUNT; ++i) {
        params_apply_value(synth, i, DEFS[i].def_val);
    }
}

bool params_state_save(const GranularSynth *synth, const clap_ostream_t *stream) {
    if (!synth || !stream) return false;
    int64_t written = stream->write(stream, synth->values, sizeof(synth->values));
    return written == sizeof(synth->values);
}

bool params_state_load(GranularSynth *synth, const clap_istream_t *stream) {
    if (!synth || !stream) return false;
    double loaded[PARAM_COUNT];
    int64_t read = stream->read(stream, loaded, sizeof(loaded));
    if (read == sizeof(loaded)) {
        for (int i = 0; i < PARAM_COUNT; ++i) {
            params_apply_value(synth, i, loaded[i]);
        }
        return true;
    }
    return false;
}
