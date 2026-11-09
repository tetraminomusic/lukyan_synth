#include "ports.h"
#include <string.h>

static uint32_t audio_ports_count(const clap_plugin_t *plugin, bool is_input) {
    (void)plugin;
    return is_input ? 0 : 1;
}

static bool audio_ports_get(const clap_plugin_t *plugin, uint32_t index, bool is_input, clap_audio_port_info_t *info) {
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

const clap_plugin_audio_ports_t g_audio_ports = {
    .count = audio_ports_count,
    .get = audio_ports_get,
};

static uint32_t note_ports_count(const clap_plugin_t *plugin, bool is_input) {
    (void)plugin;
    return is_input ? 1 : 0;
}

static bool note_ports_get(const clap_plugin_t *plugin, uint32_t index, bool is_input, clap_note_port_info_t *info) {
    (void)plugin;
    if (!info || !is_input || index > 0) return false;

    memset(info, 0, sizeof(*info));
    info->id = 0;
    strncpy(info->name, "Note Input", sizeof(info->name));
    info->supported_dialects = CLAP_NOTE_DIALECT_CLAP | CLAP_NOTE_DIALECT_MIDI;
    info->preferred_dialect = CLAP_NOTE_DIALECT_CLAP;
    return true;
}

const clap_plugin_note_ports_t g_note_ports = {
    .count = note_ports_count,
    .get = note_ports_get,
};