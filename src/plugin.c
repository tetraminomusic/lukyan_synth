#include <clap/clap.h>
#include <clap/ext/audio-ports.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265358979323846

typedef struct {
    clap_plugin_t plugin; 
    const clap_host_t* host;

    double sample_rate;
    float phase;
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

static bool plugin_activate(const struct clap_plugin *plugin,
                            double sample_rate,
                            uint32_t min_frames_count,
                            uint32_t max_frames_count) {
    (void)min_frames_count;
    (void)max_frames_count;
    GranularSynth* synth = (GranularSynth*)plugin->plugin_data;
    synth->sample_rate = sample_rate;
    synth->phase = 0.0f;
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
    synth->phase = 0.0f;
}

static clap_process_status plugin_process(const struct clap_plugin *plugin,
                                          const clap_process_t *process) {
    GranularSynth* synth = (GranularSynth*)plugin->plugin_data;

    if (process->audio_outputs_count == 0) {
        return CLAP_PROCESS_CONTINUE;
    }

    const uint32_t frame_count = process->frames_count;
    const uint32_t out_channels = process->audio_outputs[0].channel_count;

    float *out_l = (out_channels > 0) ? process->audio_outputs[0].data32[0] : NULL;
    float *out_r = (out_channels > 1) ? process->audio_outputs[0].data32[1] : NULL;

    const float freq = 440.0f; 
    const float phase_inc = (synth->sample_rate > 0.0) ? (freq / synth->sample_rate) : 0.0f;

    for (uint32_t i = 0; i < frame_count; ++i) {
        float sample = sinf(synth->phase * 2.0f * PI) * 0.1f; 
        
        synth->phase += phase_inc;
        if (synth->phase >= 1.0f) synth->phase -= 1.0f;

        if (out_l) out_l[i] = sample;
        if (out_r) out_r[i] = sample;
    }

    return CLAP_PROCESS_CONTINUE;
}

static const void* plugin_get_extension(const struct clap_plugin *plugin, const char *id) {
    (void)plugin;
    if (strcmp(id, CLAP_EXT_AUDIO_PORTS) == 0) {
        return &audio_ports;
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