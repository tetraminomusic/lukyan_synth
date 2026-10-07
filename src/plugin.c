#include <clap/clap.h>
#include <clap/ext/params.h>
#include <clap/ext/state.h>
#include <string.h>
#include <stdlib.h>

#include "parameters.h"
#include "ports.h"
#include "events.h"

static uint32_t plugin_params_count(const clap_plugin_t *plugin) {
    (void)plugin;
    return params_get_count();
}

static bool plugin_params_get_info(const clap_plugin_t *plugin, uint32_t index, clap_param_info_t *info) {
    (void)plugin;
    return params_get_info(index, info);
}

static bool plugin_params_get_value(const clap_plugin_t *plugin, clap_id param_id, double *out_value) {
    if (!plugin || !plugin->plugin_data) return false;
    return params_get_value((const GranularSynth *)plugin->plugin_data, param_id, out_value);
}

static bool plugin_params_value_to_text(const clap_plugin_t *plugin, clap_id param_id, double value, char *out_buffer, uint32_t out_buffer_capacity) {
    (void)plugin;
    return params_value_to_text(param_id, value, out_buffer, out_buffer_capacity);
}

static const clap_plugin_params_t params_ext = {
    .count = plugin_params_count,
    .get_info = plugin_params_get_info,
    .get_value = plugin_params_get_value,
    .value_to_text = plugin_params_value_to_text,
    .text_to_value = NULL,
    .flush = NULL,
};

static bool plugin_state_save(const clap_plugin_t *plugin, const clap_ostream_t *stream) {
    if (!plugin || !plugin->plugin_data) return false;
    return params_state_save((const GranularSynth *)plugin->plugin_data, stream);
}

static bool plugin_state_load(const clap_plugin_t *plugin, const clap_istream_t *stream) {
    if (!plugin || !plugin->plugin_data) return false;
    return params_state_load((GranularSynth *)plugin->plugin_data, stream);
}

static const clap_plugin_state_t state_ext = {
    .save = plugin_state_save,
    .load = plugin_state_load,
};

static bool plugin_init(const struct clap_plugin *plugin) { (void)plugin; return true; }

static void plugin_destroy(const struct clap_plugin *plugin) {
    if (!plugin) return;
    GranularSynth *synth = (GranularSynth *)plugin->plugin_data;
    if (synth) free(synth);
}

static bool plugin_activate(const struct clap_plugin *plugin, double sample_rate, uint32_t min_f, uint32_t max_f) {
    (void)min_f; (void)max_f;
    if (!plugin || !plugin->plugin_data) return false;
    GranularSynth *synth = (GranularSynth *)plugin->plugin_data;

    granular_init(&synth->engine, sample_rate);
    params_init_defaults(synth);
    return true;
}

static void plugin_deactivate(const struct clap_plugin *plugin) { (void)plugin; }
static bool plugin_start_processing(const struct clap_plugin *plugin) { (void)plugin; return true; }
static void plugin_stop_processing(const struct clap_plugin *plugin) { (void)plugin; }

static void plugin_reset(const struct clap_plugin *plugin) {
    if (!plugin || !plugin->plugin_data) return;
    granular_reset(&((GranularSynth *)plugin->plugin_data)->engine);
}

static clap_process_status plugin_process(const struct clap_plugin *plugin, const clap_process_t *process) {
    if (!plugin || !plugin->plugin_data || !process) return CLAP_PROCESS_CONTINUE;
    GranularSynth *synth = (GranularSynth *)plugin->plugin_data;

    events_process_input(synth, process->in_events);

    if (process->audio_outputs_count == 0) return CLAP_PROCESS_CONTINUE;

    float *out_l = (process->audio_outputs[0].channel_count > 0) ? process->audio_outputs[0].data32[0] : NULL;
    float *out_r = (process->audio_outputs[0].channel_count > 1) ? process->audio_outputs[0].data32[1] : NULL;

    granular_render_block(&synth->engine, out_l, out_r, process->frames_count);

    return CLAP_PROCESS_CONTINUE;
}

static const void* plugin_get_extension(const struct clap_plugin *plugin, const char *id) {
    (void)plugin;
    if (!id) return NULL;
    if (strcmp(id, CLAP_EXT_AUDIO_PORTS) == 0) return &g_audio_ports;
    if (strcmp(id, CLAP_EXT_NOTE_PORTS) == 0) return &g_note_ports;
    if (strcmp(id, CLAP_EXT_PARAMS) == 0) return &params_ext;
    if (strcmp(id, CLAP_EXT_STATE) == 0) return &state_ext;
    return NULL;
}

static void plugin_on_main_thread(const struct clap_plugin *plugin) { (void)plugin; }

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
    .description = "Polyphonic Multi-Oscillator Granular Synthesizer by tetramino",
    .features = plugin_features
};

static uint32_t factory_get_plugin_count(const struct clap_plugin_factory *factory) { (void)factory; return 1; }
static const clap_plugin_descriptor_t* factory_get_descriptor(const struct clap_plugin_factory *f, uint32_t i) {
    (void)f; (void)i; return &plugin_descriptor;
}

static const clap_plugin_t* factory_create_plugin(const struct clap_plugin_factory *f, const clap_host_t *host, const char *id) {
    (void)f;
    if (strcmp(id, plugin_descriptor.id) != 0) return NULL;

    GranularSynth *synth = (GranularSynth *)calloc(1, sizeof(GranularSynth));
    if (!synth) return NULL;

    synth->plugin = plugin_class;
    synth->plugin.desc = &plugin_descriptor;
    synth->plugin.plugin_data = synth;
    synth->host = host;

    return &synth->plugin;
}

static const clap_plugin_factory_t plugin_factory = {
    .get_plugin_count = factory_get_plugin_count,
    .get_plugin_descriptor = factory_get_descriptor,
    .create_plugin = factory_create_plugin,
};

static bool entry_init(const char *plugin_path) { (void)plugin_path; return true; }
static void entry_deinit(void) {}
static const void* entry_get_factory(const char *factory_id) {
    if (strcmp(factory_id, CLAP_PLUGIN_FACTORY_ID) == 0) return &plugin_factory;
    return NULL;
}

CLAP_EXPORT const clap_plugin_entry_t clap_entry = {
    .clap_version = CLAP_VERSION_INIT,
    .init = entry_init,
    .deinit = entry_deinit,
    .get_factory = entry_get_factory,
};