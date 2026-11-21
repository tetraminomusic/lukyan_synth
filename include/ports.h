/**
 * @file ports.h
 * @brief Описание аудио- и MIDI-портов плагина.
 *
 * Реализует расширения CLAP_EXT_AUDIO_PORTS и CLAP_EXT_NOTE_PORTS,
 * сообщая хосту о наличии одного стерео-выхода и одного MIDI-входа.
 */

#ifndef PORTS_H
#define PORTS_H

#include <clap/clap.h>
#include <clap/ext/audio-ports.h>
#include <clap/ext/note-ports.h>

/**
 * @brief Расширение CLAP_AUDIO_PORTS (1 стерео-выход, 0 входов).
 */
extern const clap_plugin_audio_ports_t g_audio_ports;

/**
 * @brief Расширение CLAP_NOTE_PORTS (1 MIDI-вход, 0 выходов).
 */
extern const clap_plugin_note_ports_t g_note_ports;

#endif