#include <clap/clap.h>
#include <clap/ext/audio-ports.h>
#include <clap/ext/note-ports.h>
#include <clap/ext/params.h>
#include <clap/ext/state.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "granular.h"

enum {
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
    PARAM_COUNT
};

typedef struct {
    clap_plugin_t plugin; 
    const clap_host_t* host;

    double grain_size;
    double density;
    double spray;
    double attack;
    double decay;
    double sustain;
    double release;
    double gain;
    double crush;
    double downsample;

    GranularEngine engine;
} GranularSynth;

static uint32_t plugin_audio_ports_count(const clap_plugin_t *plugin, bool is_input) {
    (void)plugin;
    return is_input ? 0 : 1;
}

static bool plugin_audio_ports_get(const clap_plugin_t *plugin,
                                   uint32_t index,
                                   bool is_input,
                                   clap_audio_port_info_t *info) {
    (void)plugin;
    if (!info || is_input || index > 0) return false;

    memset(info, 0, sizeof(*info));

    info->id = 0;
    strncpy(info->name, "Main Output", sizeof(info->name));
    info->channel_count = 2;
    info->flags = CLAP_AUDIO_PORT_IS_MAIN;
    info->port_type = CLAP_PORT_STEREO;
    info->in_place_pair = CLAP_INVALID_ID;
    return true;
}

static const clap_plugin_audio_ports_t audio_ports = {
    .count = plugin_audio_ports_count,
    .get = plugin_audio_ports_get,
};

static uint32_t plugin_note_ports_count(const clap_plugin_t *plugin, bool is_input) {
    (void)plugin;
    return is_input ? 1 : 0;
}

static bool plugin_note_ports_get(const clap_plugin_t *plugin,
                                  uint32_t index,
                                  bool is_input,
                                  clap_note_port_info_t *info) {
    (void)plugin;
    if (!info || !is_input || index > 0) return false;

    memset(info, 0, sizeof(*info));

    info->id = 0;
    strncpy(info->name, "Note Input", sizeof(info->name));
    info->supported_dialects = CLAP_NOTE_DIALECT_CLAP | CLAP_NOTE_DIALECT_MIDI;
    info->preferred_dialect = CLAP_NOTE_DIALECT_CLAP;
    return true;
}

static const clap_plugin_note_ports_t note_ports = {
    .count = plugin_note_ports_count,
    .get = plugin_note_ports_get,
};

static uint32_t plugin_params_count(const clap_plugin_t *plugin) {
    (void)plugin;
    return PARAM_COUNT;
}

static bool plugin_params_get_info(const clap_plugin_t *plugin, uint32_t index, clap_param_info_t *info) {
    (void)plugin;
    if (!info) return false;

    memset(info, 0, sizeof(*info));

    switch (index) {
        case PARAM_GRAIN_SIZE:
            info->id = PARAM_GRAIN_SIZE;
            info->flags = CLAP_PARAM_IS_AUTOMATABLE;
            strncpy(info->name, "Grain Size", sizeof(info->name));
            info->min_value = 10.0;
            info->max_value = 200.0;
            info->default_value = 65.0;
            return true;

        case PARAM_DENSITY:
            info->id = PARAM_DENSITY;
            info->flags = CLAP_PARAM_IS_AUTOMATABLE;
            strncpy(info->name, "Density", sizeof(info->name));
            info->min_value = 5.0;
            info->max_value = 100.0;
            info->default_value = 30.0;
            return true;

        case PARAM_SPRAY:
            info->id = PARAM_SPRAY;
            info->flags = CLAP_PARAM_IS_AUTOMATABLE;
            strncpy(info->name, "Spray", sizeof(info->name));
            info->min_value = 0.0;
            info->max_value = 1.0;
            info->default_value = 0.15;
            return true;

        case PARAM_ATTACK:
            info->id = PARAM_ATTACK;
            info->flags = CLAP_PARAM_IS_AUTOMATABLE;
            strncpy(info->name, "Attack", sizeof(info->name));
            info->min_value = 1.0;
            info->max_value = 2000.0;
            info->default_value = 20.0;
            return true;

        case PARAM_DECAY:
            info->id = PARAM_DECAY;
            info->flags = CLAP_PARAM_IS_AUTOMATABLE;
            strncpy(info->name, "Decay", sizeof(info->name));
            info->min_value = 10.0;
            info->max_value = 2000.0;
            info->default_value = 200.0;
            return true;

        case PARAM_SUSTAIN:
            info->id = PARAM_SUSTAIN;
            info->flags = CLAP_PARAM_IS_AUTOMATABLE;
            strncpy(info->name, "Sustain", sizeof(info->name));
            info->min_value = 0.0;
            info->max_value = 1.0;
            info->default_value = 0.75;
            return true;

        case PARAM_RELEASE:
            info->id = PARAM_RELEASE;
            info->flags = CLAP_PARAM_IS_AUTOMATABLE;
            strncpy(info->name, "Release", sizeof(info->name));
            info->min_value = 10.0;
            info->max_value = 3000.0;
            info->default_value = 350.0;
            return true;

        case PARAM_GAIN:
            info->id = PARAM_GAIN;
            info->flags = CLAP_PARAM_IS_AUTOMATABLE;
            strncpy(info->name, "Master Gain", sizeof(info->name));
            info->min_value = 0.0;
            info->max_value = 1.0;
            info->default_value = 0.75;
            return true;

        case PARAM_CRUSH:
            info->id = PARAM_CRUSH;
            info->flags = CLAP_PARAM_IS_AUTOMATABLE;
            strncpy(info->name, "Lo-Fi Bits", sizeof(info->name));
            info->min_value = 4.0;
            info->max_value = 16.0;
            info->default_value = 16.0;
            return true;

        case PARAM_DOWNSAMPLE:
            info->id = PARAM_DOWNSAMPLE;
            info->flags = CLAP_PARAM_IS_AUTOMATABLE;
            strncpy(info->name, "Lo-Fi Rate", sizeof(info->name));
            info->min_value = 1.0;
            info->max_value = 24.0;
            info->default_value = 1.0;
            return true;

        default:
            return false;
    }
}

static bool plugin_params_get_value(const clap_plugin_t *plugin, clap_id param_id, double *out_value) {
    if (!plugin || !plugin->plugin_data || !out_value) return false;
    GranularSynth *synth = (GranularSynth *)plugin->plugin_data;

    switch (param_id) {
        case PARAM_GRAIN_SIZE: *out_value = synth->grain_size; return true;
        case PARAM_DENSITY:    *out_value = synth->density; return true;
        case PARAM_SPRAY:      *out_value = synth->spray; return true;
        case PARAM_ATTACK:     *out_value = synth->attack; return true;
        case PARAM_DECAY:      *out_value = synth->decay; return true;
        case PARAM_SUSTAIN:    *out_value = synth->sustain; return true;
        case PARAM_RELEASE:    *out_value = synth->release; return true;
        case PARAM_GAIN:       *out_value = synth->gain; return true;
        case PARAM_CRUSH:      *out_value = synth->crush; return true;
        case PARAM_DOWNSAMPLE: *out_value = synth->downsample; return true;
        default: return false;
    }
}

static bool plugin_params_value_to_text(const clap_plugin_t *plugin, clap_id param_id, double value, char *out_buffer, uint32_t out_buffer_capacity) {
    (void)plugin;
    if (!out_buffer || out_buffer_capacity == 0) return false;

    switch (param_id) {
        case PARAM_GRAIN_SIZE: snprintf(out_buffer, out_buffer_capacity, "%.1f ms", value); return true;
        case PARAM_DENSITY:    snprintf(out_buffer, out_buffer_capacity, "%.1f gr/s", value); return true;
        case PARAM_SPRAY:      snprintf(out_buffer, out_buffer_capacity, "%.2f", value); return true;
        case PARAM_ATTACK:     snprintf(out_buffer, out_buffer_capacity, "%.1f ms", value); return true;
        case PARAM_DECAY:      snprintf(out_buffer, out_buffer_capacity, "%.1f ms", value); return true;
        case PARAM_SUSTAIN:    snprintf(out_buffer, out_buffer_capacity, "%.2f", value); return true;
        case PARAM_RELEASE:    snprintf(out_buffer, out_buffer_capacity, "%.1f ms", value); return true;
        case PARAM_GAIN:       snprintf(out_buffer, out_buffer_capacity, "%.0f %%", value * 100.0); return true;
        case PARAM_CRUSH:      snprintf(out_buffer, out_buffer_capacity, "%.1f bit", value); return true;
        case PARAM_DOWNSAMPLE: snprintf(out_buffer, out_buffer_capacity, "%.0fx", value); return true;
        default: return false;
    }
}

static bool plugin_params_text_to_value(const clap_plugin_t *plugin, clap_id param_id, const char *param_value_text, double *out_value) {
    (void)plugin; (void)param_id; (void)param_value_text; (void)out_value;
    return false;
}

static void apply_param_value(GranularSynth *synth, clap_id param_id, double value) {
    switch (param_id) {
        case PARAM_GRAIN_SIZE:
            synth->grain_size = value;
            granular_set_grain_size(&synth->engine, (float)value);
            break;
        case PARAM_DENSITY:
            synth->density = value;
            granular_set_density(&synth->engine, (float)value);
            break;
        case PARAM_SPRAY:
            synth->spray = value;
            granular_set_spray(&synth->engine, (float)value);
            break;
        case PARAM_ATTACK:
            synth->attack = value;
            granular_set_attack(&synth->engine, (float)value);
            break;
        case PARAM_DECAY:
            synth->decay = value;
            granular_set_decay(&synth->engine, (float)value);
            break;
        case PARAM_SUSTAIN:
            synth->sustain = value;
            granular_set_sustain(&synth->engine, (float)value);
            break;
        case PARAM_RELEASE:
            synth->release = value;
            granular_set_release(&synth->engine, (float)value);
            break;
        case PARAM_GAIN:
            synth->gain = value;
            granular_set_gain(&synth->engine, (float)value);
            break;
        case PARAM_CRUSH:
            synth->crush = value;
            granular_set_crush(&synth->engine, (float)value);
            break;
        case PARAM_DOWNSAMPLE:
            synth->downsample = value;
            granular_set_downsample(&synth->engine, (float)value);
            break;
    }
}

static void plugin_params_flush(const clap_plugin_t *plugin, const clap_input_events_t *in, const clap_output_events_t *out) {
    (void)out;
    if (!plugin || !plugin->plugin_data || !in) return;
    GranularSynth *synth = (GranularSynth *)plugin->plugin_data;

    uint32_t size = in->size(in);
    for (uint32_t i = 0; i < size; ++i) {
        const clap_event_header_t *hdr = in->get(in, i);
        if (!hdr || hdr->space_id != CLAP_CORE_EVENT_SPACE_ID) continue;

        if (hdr->type == CLAP_EVENT_PARAM_VALUE) {
            const clap_event_param_value_t *ev = (const clap_event_param_value_t *)hdr;
            apply_param_value(synth, ev->param_id, ev->value);
        }
    }
}

static const clap_plugin_params_t params_ext = {
    .count = plugin_params_count,
    .get_info = plugin_params_get_info,
    .get_value = plugin_params_get_value,
    .value_to_text = plugin_params_value_to_text,
    .text_to_value = plugin_params_text_to_value,
    .flush = plugin_params_flush,
};

static bool plugin_state_save(const clap_plugin_t *plugin, const clap_ostream_t *stream) {
    if (!plugin || !plugin->plugin_data || !stream) return false;
    GranularSynth *synth = (GranularSynth *)plugin->plugin_data;

    double state_data[PARAM_COUNT] = {
        synth->grain_size,
        synth->density,
        synth->spray,
        synth->attack,
        synth->decay,
        synth->sustain,
        synth->release,
        synth->gain,
        synth->crush,
        synth->downsample
    };
    int64_t written = stream->write(stream, state_data, sizeof(state_data));
    return written == sizeof(state_data);
}

static bool plugin_state_load(const clap_plugin_t *plugin, const clap_istream_t *stream) {
    if (!plugin || !plugin->plugin_data || !stream) return false;
    GranularSynth *synth = (GranularSynth *)plugin->plugin_data;

    double state_data[PARAM_COUNT] = { 0 };
    int64_t read = stream->read(stream, state_data, sizeof(state_data));
    if (read == sizeof(state_data)) {
        for (int i = 0; i < PARAM_COUNT; ++i) {
            apply_param_value(synth, i, state_data[i]);
        }
        return true;
    }
    return false;
}

static const clap_plugin_state_t state_ext = {
    .save = plugin_state_save,
    .load = plugin_state_load,
};

static bool plugin_init(const struct clap_plugin *plugin) {
    (void)plugin;
    return true; 
}

static void plugin_destroy(const struct clap_plugin *plugin) {
    if (!plugin) return;
    GranularSynth* synth = (GranularSynth*)plugin->plugin_data;
    if (synth) {
        free(synth);
    }
}

static bool plugin_activate(const struct clap_plugin *plugin,
                            double sample_rate,
                            uint32_t min_frames_count,
                            uint32_t max_frames_count) {
    (void)min_frames_count;
    (void)max_frames_count;
    if (!plugin || !plugin->plugin_data) return false;
    GranularSynth* synth = (GranularSynth*)plugin->plugin_data;
    
    synth->grain_size = 65.0;
    synth->density = 30.0;
    synth->spray = 0.15;
    synth->attack = 20.0;
    synth->decay = 200.0;
    synth->sustain = 0.75;
    synth->release = 350.0;
    synth->gain = 0.75;
    synth->crush = 16.0;
    synth->downsample = 1.0;

    granular_init(&synth->engine, sample_rate);
    for (int i = 0; i < PARAM_COUNT; ++i) {
        double val = 0.0;
        plugin_params_get_value(plugin, i, &val);
        apply_param_value(synth, i, val);
    }
    return true;
}

static void plugin_deactivate(const struct clap_plugin *plugin) {
    (void)plugin;
}

static bool plugin_start_processing(const struct clap_plugin *plugin) {
    (void)plugin;
    return true;
}

static void plugin_stop_processing(const struct clap_plugin *plugin) {
    (void)plugin;
}

static void plugin_reset(const struct clap_plugin *plugin) {
    if (!plugin || !plugin->plugin_data) return;
    GranularSynth* synth = (GranularSynth*)plugin->plugin_data;
    granular_reset(&synth->engine);
}

static void process_input_events(GranularSynth *synth, const clap_input_events_t *in_events) {
    if (!in_events) return;

    const uint32_t event_count = in_events->size(in_events);
    for (uint32_t i = 0; i < event_count; ++i) {
        const clap_event_header_t *hdr = in_events->get(in_events, i);
        if (!hdr || hdr->space_id != CLAP_CORE_EVENT_SPACE_ID) continue;

        if (hdr->type == CLAP_EVENT_NOTE_ON) {
            const clap_event_note_t *note = (const clap_event_note_t *)hdr;
            float freq = 440.0f * powf(2.0f, (float)(note->key - 69) / 12.0f);
            granular_note_on(&synth->engine, note->key, freq);
        } else if (hdr->type == CLAP_EVENT_NOTE_OFF) {
            const clap_event_note_t *note = (const clap_event_note_t *)hdr;
            granular_note_off(&synth->engine, note->key);
        } else if (hdr->type == CLAP_EVENT_PARAM_VALUE) {
            const clap_event_param_value_t *ev = (const clap_event_param_value_t *)hdr;
            apply_param_value(synth, ev->param_id, ev->value);
        } else if (hdr->type == CLAP_EVENT_MIDI) {
            const clap_event_midi_t *midi = (const clap_event_midi_t *)hdr;
            uint8_t status = midi->data[0] & 0xF0;
            if (status == 0xE0) {
                int bend_raw = (midi->data[1] & 0x7F) | ((midi->data[2] & 0x7F) << 7);
                float bend_semitones = ((float)(bend_raw - 8192) / 8192.0f) * 2.0f;
                granular_set_pitch_bend(&synth->engine, bend_semitones);
            }
        }
    }
}

static clap_process_status plugin_process(const struct clap_plugin *plugin,
                                          const clap_process_t *process) {
    if (!plugin || !plugin->plugin_data || !process) return CLAP_PROCESS_CONTINUE;
    GranularSynth* synth = (GranularSynth*)plugin->plugin_data;

    process_input_events(synth, process->in_events);

    if (process->audio_outputs_count == 0) {
        return CLAP_PROCESS_CONTINUE;
    }

    const uint32_t frame_count = process->frames_count;
    const uint32_t out_channels = process->audio_outputs[0].channel_count;

    float *out_l = (out_channels > 0) ? process->audio_outputs[0].data32[0] : NULL;
    float *out_r = (out_channels > 1) ? process->audio_outputs[0].data32[1] : NULL;

    for (uint32_t i = 0; i < frame_count; ++i) {
        float sample_l = 0.0f;
        float sample_r = 0.0f;

        granular_render_sample(&synth->engine, &sample_l, &sample_r);

        if (out_l) out_l[i] = sample_l;
        if (out_r) out_r[i] = sample_r;
    }

    return CLAP_PROCESS_CONTINUE;
}

static const void* plugin_get_extension(const struct clap_plugin *plugin, const char *id) {
    (void)plugin;
    if (!id) return NULL;

    if (strcmp(id, CLAP_EXT_AUDIO_PORTS) == 0) return &audio_ports;
    if (strcmp(id, CLAP_EXT_NOTE_PORTS) == 0) return &note_ports;
    if (strcmp(id, CLAP_EXT_PARAMS) == 0) return &params_ext;
    if (strcmp(id, CLAP_EXT_STATE) == 0) return &state_ext;
    return NULL;
}

static void plugin_on_main_thread(const struct clap_plugin *plugin) {
    (void)plugin;
}

static const clap_plugin_t plugin_class = {
    .desc = NULL,
    .plugin_data = NULL,
    .init = plugin_init,
    .destroy = plugin_destroy,
    .activate = plugin_activate,
    .deactivate = plugin_deactivate,
    .start_processing = plugin_start_processing,
    .stop_processing = plugin_stop_processing,
    .reset = plugin_reset,
    .process = plugin_process,
    .get_extension = plugin_get_extension,
    .on_main_thread = plugin_on_main_thread,
};

static const char *plugin_features[] = {
    CLAP_PLUGIN_FEATURE_INSTRUMENT,
    CLAP_PLUGIN_FEATURE_SYNTHESIZER,
    CLAP_PLUGIN_FEATURE_STEREO,
    NULL
};

static const clap_plugin_descriptor_t plugin_descriptor = {
    .clap_version = CLAP_VERSION_INIT,
    .id = "com.tetramino.lukyansynth", 
    .name = "Lukyan Synth", 
    .vendor = "tetramino", 
    .url = NULL,
    .manual_url = NULL,
    .support_url = NULL,
    .version = "1.0.0",
    .description = "Великий и богоподный полифонический синтезатор с гранулярками by tetramino",
    .features = plugin_features
};

static uint32_t plugin_factory_get_plugin_count(const struct clap_plugin_factory *factory) {
    (void)factory;
    return 1;
}

static const clap_plugin_descriptor_t* plugin_factory_get_plugin_descriptor(const struct clap_plugin_factory *factory, uint32_t index) {
    (void)factory;
    (void)index;
    return &plugin_descriptor;
}

static const clap_plugin_t* plugin_factory_create_plugin(const struct clap_plugin_factory *factory, const clap_host_t *host, const char *plugin_id) {
    (void)factory;
    if (strcmp(plugin_id, plugin_descriptor.id) != 0) return NULL;

    GranularSynth* synth = (GranularSynth*)calloc(1, sizeof(GranularSynth));
    if (!synth) return NULL;

    synth->plugin = plugin_class;
    synth->plugin.desc = &plugin_descriptor;
    synth->plugin.plugin_data = synth;
    synth->host = host;

    return &synth->plugin;
}

static const clap_plugin_factory_t plugin_factory = {
    .get_plugin_count = plugin_factory_get_plugin_count,
    .get_plugin_descriptor = plugin_factory_get_plugin_descriptor,
    .create_plugin = plugin_factory_create_plugin,
};

static bool entry_init(const char *plugin_path) {
    (void)plugin_path;
    return true;
}

static void entry_deinit(void) {}

static const void* entry_get_factory(const char *factory_id) {
    if (strcmp(factory_id, CLAP_PLUGIN_FACTORY_ID) == 0) {
        return &plugin_factory;
    }
    return NULL;
}

CLAP_EXPORT const clap_plugin_entry_t clap_entry = {
    .clap_version = CLAP_VERSION_INIT,
    .init = entry_init,
    .deinit = entry_deinit,
    .get_factory = entry_get_factory,
};