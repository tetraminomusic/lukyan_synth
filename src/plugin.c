#include <clap/clap.h>
#include <clap/ext/audio-ports.h>
#include <clap/ext/note-ports.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265358979323846
#define MAX_GRAINS 64
#define HANN_LUT_SIZE 2048
#define SAMPLE_BUFFER_SIZE 96000

typedef struct {
    bool active;
    float pos;
    float speed;
    float length;
    float progress;
} Grain;

typedef struct {
    clap_plugin_t plugin; 
    const clap_host_t* host;

    double sample_rate;
    float frequency;
    bool is_note_on;
    int32_t current_key;

    float sample_buffer[SAMPLE_BUFFER_SIZE];
    float hann_lut[HANN_LUT_SIZE];
    Grain grains[MAX_GRAINS];

    float spawn_timer;
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
    if (is_input || index > 0) return false;

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
    if (!is_input || index > 0) return false;

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

static bool plugin_init(const struct clap_plugin *plugin) {
    (void)plugin;
    return true; 
}

static void plugin_destroy(const struct clap_plugin *plugin) {
    GranularSynth* synth = (GranularSynth*)plugin->plugin_data;
    if (synth) {
        free(synth);
    }
}

static void init_hann_lut(GranularSynth *synth) {
    for (int i = 0; i < HANN_LUT_SIZE; ++i) {
        synth->hann_lut[i] = 0.5f * (1.0f - cosf((2.0f * (float)PI * (float)i) / (float)(HANN_LUT_SIZE - 1)));
    }
}

static void init_sample_buffer(GranularSynth *synth) {
    for (int i = 0; i < SAMPLE_BUFFER_SIZE; ++i) {
        float t = (float)i / (float)SAMPLE_BUFFER_SIZE;
        float wave = sinf(2.0f * (float)PI * 220.0f * t) * 0.5f
                   + sinf(2.0f * (float)PI * 440.0f * t) * 0.25f
                   + sinf(2.0f * (float)PI * 880.0f * t) * 0.125f;
        synth->sample_buffer[i] = wave;
    }
}

static bool plugin_activate(const struct clap_plugin *plugin,
                            double sample_rate,
                            uint32_t min_frames_count,
                            uint32_t max_frames_count) {
    (void)min_frames_count;
    (void)max_frames_count;
    GranularSynth* synth = (GranularSynth*)plugin->plugin_data;
    synth->sample_rate = sample_rate;
    synth->frequency = 440.0f;
    synth->is_note_on = false;
    synth->current_key = -1;
    synth->spawn_timer = 0.0f;

    init_hann_lut(synth);
    init_sample_buffer(synth);

    for (int i = 0; i < MAX_GRAINS; ++i) {
        synth->grains[i].active = false;
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
    GranularSynth* synth = (GranularSynth*)plugin->plugin_data;
    synth->is_note_on = false;
    synth->current_key = -1;
    synth->spawn_timer = 0.0f;
    for (int i = 0; i < MAX_GRAINS; ++i) {
        synth->grains[i].active = false;
    }
}

static void spawn_grain(GranularSynth *synth) {
    for (int i = 0; i < MAX_GRAINS; ++i) {
        if (!synth->grains[i].active) {
            synth->grains[i].active = true;
            synth->grains[i].pos = (float)(rand() % (SAMPLE_BUFFER_SIZE / 2));
            synth->grains[i].speed = synth->frequency / 220.0f;
            synth->grains[i].length = (float)synth->sample_rate * 0.08f;
            synth->grains[i].progress = 0.0f;
            break;
        }
    }
}

static void process_input_events(GranularSynth *synth, const clap_input_events_t *in_events) {
    if (!in_events) return;

    const uint32_t event_count = in_events->size(in_events);
    for (uint32_t i = 0; i < event_count; ++i) {
        const clap_event_header_t *hdr = in_events->get(in_events, i);
        if (hdr->space_id != CLAP_CORE_EVENT_SPACE_ID) continue;

        if (hdr->type == CLAP_EVENT_NOTE_ON) {
            const clap_event_note_t *note = (const clap_event_note_t *)hdr;
            synth->current_key = note->key;
            synth->frequency = 440.0f * powf(2.0f, (float)(note->key - 69) / 12.0f);
            synth->is_note_on = true;
        } else if (hdr->type == CLAP_EVENT_NOTE_OFF) {
            const clap_event_note_t *note = (const clap_event_note_t *)hdr;
            if (synth->current_key == note->key) {
                synth->is_note_on = false;
                synth->current_key = -1;
            }
        }
    }
}

static clap_process_status plugin_process(const struct clap_plugin *plugin,
                                          const clap_process_t *process) {
    GranularSynth* synth = (GranularSynth*)plugin->plugin_data;

    process_input_events(synth, process->in_events);

    if (process->audio_outputs_count == 0) {
        return CLAP_PROCESS_CONTINUE;
    }

    const uint32_t frame_count = process->frames_count;
    const uint32_t out_channels = process->audio_outputs[0].channel_count;

    float *out_l = (out_channels > 0) ? process->audio_outputs[0].data32[0] : NULL;
    float *out_r = (out_channels > 1) ? process->audio_outputs[0].data32[1] : NULL;

    const float spawn_interval = (synth->sample_rate > 0.0) ? ((float)synth->sample_rate / 40.0f) : 1000.0f;

    for (uint32_t i = 0; i < frame_count; ++i) {
        if (synth->is_note_on) {
            synth->spawn_timer += 1.0f;
            if (synth->spawn_timer >= spawn_interval) {
                synth->spawn_timer = 0.0f;
                spawn_grain(synth);
            }
        }

        float mixed_sample = 0.0f;

        for (int g = 0; g < MAX_GRAINS; ++g) {
            if (!synth->grains[g].active) continue;

            Grain *grain = &synth->grains[g];

            float win_norm = grain->progress / grain->length;
            int lut_idx = (int)(win_norm * (float)(HANN_LUT_SIZE - 1));
            if (lut_idx >= HANN_LUT_SIZE) lut_idx = HANN_LUT_SIZE - 1;
            float env = synth->hann_lut[lut_idx];

            int idx_a = (int)grain->pos;
            int idx_b = (idx_a + 1) % SAMPLE_BUFFER_SIZE;
            float frac = grain->pos - (float)idx_a;
            float audio_val = synth->sample_buffer[idx_a] * (1.0f - frac) + synth->sample_buffer[idx_b] * frac;

            mixed_sample += audio_val * env * 0.15f;

            grain->pos += grain->speed;
            if (grain->pos >= (float)SAMPLE_BUFFER_SIZE) {
                grain->pos -= (float)SAMPLE_BUFFER_SIZE;
            }

            grain->progress += 1.0f;
            if (grain->progress >= grain->length) {
                grain->active = false;
            }
        }

        if (out_l) out_l[i] = mixed_sample;
        if (out_r) out_r[i] = mixed_sample;
    }

    return CLAP_PROCESS_CONTINUE;
}

static const void* plugin_get_extension(const struct clap_plugin *plugin, const char *id) {
    (void)plugin;
    if (strcmp(id, CLAP_EXT_AUDIO_PORTS) == 0) {
        return &audio_ports;
    }
    if (strcmp(id, CLAP_EXT_NOTE_PORTS) == 0) {
        return &note_ports;
    }
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
    .features = (const char*[]){ 
        CLAP_PLUGIN_FEATURE_INSTRUMENT, 
        CLAP_PLUGIN_FEATURE_SYNTHESIZER, 
        CLAP_PLUGIN_FEATURE_STEREO,
        NULL 
    }
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