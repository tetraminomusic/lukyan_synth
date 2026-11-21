/**
 * @file presets.h
 * @brief Менеджер пресетов с человекочитаемым текстовым форматом.
 *
 * Пресеты сохраняются в формате `.lukyan` — plain-text файл, где
 * каждая строка содержит ID параметра и его значение.
 */

#ifndef PRESETS_H
#define PRESETS_H

#include "parameters.h"
#include <stdbool.h>

#define NUM_FACTORY_PRESETS 4    /**< Количество встроенных пресетов (устарело). */
#define NUM_USER_PRESETS 5       /**< Количество слотов пользователя (устарело). */

/**
 * @brief Структура одного пресета.
 */
typedef struct {
    char name[32];               /**< Имя пресета (до 31 символа). */
    double values[PARAM_COUNT];  /**< Массив значений всех параметров. */
} Preset;

extern Preset g_factory_presets[NUM_FACTORY_PRESETS]; /**< Массив фабричных пресетов. */

/**
 * @brief Инициализирует встроенные пресеты (устарело, оставлено для совместимости).
 * @param synth Указатель на экземпляр синтезатора.
 */
void presets_init_factory(GranularSynth *synth);

/**
 * @brief Применяет пресет к движку синтезатора.
 * @param synth  Указатель на экземпляр синтезатора.
 * @param preset Указатель на пресет.
 */
void presets_load_state(GranularSynth *synth, const Preset *preset);

/**
 * @brief Сохраняет текущее состояние в текстовый файл пресета.
 * @param synth    Указатель на экземпляр синтезатора.
 * @param filepath Путь к файлу для сохранения.
 * @return true при успехе, false при ошибке.
 */
bool presets_save_to_file(const GranularSynth *synth, const char *filepath);

/**
 * @brief Загружает состояние из текстового файла пресета.
 * @param synth    Указатель на экземпляр синтезатора.
 * @param filepath Путь к файлу для загрузки.
 * @return true при успехе, false при ошибке.
 */
bool presets_load_from_file(GranularSynth *synth, const char *filepath);

#endif