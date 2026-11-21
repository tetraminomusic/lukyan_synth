/**
 * @file events.h
 * @brief Диспетчер входных событий плагина.
 *
 * Обрабатывает входящий поток CLAP-событий от хоста: MIDI Note On/Off,
 * Pitch Bend, автоматизацию параметров.
 */

#ifndef EVENTS_H
#define EVENTS_H

#include "parameters.h"

/**
 * @brief Обрабатывает все входные события из текущего аудио-блока.
 *
 * Поддерживаемые типы событий:
 *   - CLAP_EVENT_NOTE_ON      — запускает голос (ADSR в ATTACK)
 *   - CLAP_EVENT_NOTE_OFF     — переводит голос в RELEASE
 *   - CLAP_EVENT_PARAM_VALUE  — применяет значение параметра (автоматизация)
 *   - CLAP_EVENT_MIDI         — парсит Pitch Bend из MIDI-сообщений (0xE0)
 *
 * @param synth     Указатель на экземпляр синтезатора.
 * @param in_events Указатель на очередь входных событий (может быть NULL).
 */
void events_process_input(GranularSynth *synth, const clap_input_events_t *in_events);

#endif