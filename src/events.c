#include "events.h"
#include <math.h>

void events_process_input(GranularSynth *synth, const clap_input_events_t *in_events) {
    if (!synth || !in_events) return;

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
            params_apply_value(synth, ev->param_id, ev->value);
        } else if (hdr->type == CLAP_EVENT_MIDI) {
            const clap_event_midi_t *midi = (const clap_event_midi_t *)hdr;
            if ((midi->data[0] & 0xF0) == 0xE0) {
                int bend_raw = (midi->data[1] & 0x7F) | ((midi->data[2] & 0x7F) << 7);
                float bend_semi = ((float)(bend_raw - 8192) / 8192.0f) * 2.0f;
                granular_set_pitch_bend(&synth->engine, bend_semi);
            }
        }
    }
}