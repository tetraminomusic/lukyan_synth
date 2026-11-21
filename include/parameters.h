/**
 * @file parameters.h
 * @brief Управление параметрами плагина.
 *
 * Содержит таблицу всех параметров, экспортируемых в DAW через
 * расширение CLAP_EXT_PARAMS.
 * 
 */

#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <clap/clap.h>
#include "granular.h"
#include "gui.h"
#include "arena.h"

#define PLUGIN_MEMORY_POOL_SIZE (4 * 1024 * 1024) /**< Размер арены памяти */

/**
 * @brief Уникальные идентификаторы всех параметров плагина.
 *
 * Порядок в enum должен строго соответствовать порядку в таблице DEFS
 * в src/dsp/parameters.c.
 */
typedef enum {
    PARAM_GRAIN_SIZE = 0,                               /**< Длина зерна*/
    PARAM_DENSITY,                                      /**< Плотность зёрен*/
    PARAM_SPRAY,                                        /**< Разброс позиции старта зерна. */
    PARAM_ATTACK,                                       /**< ADSR: Attack*/
    PARAM_DECAY,                                        /**< ADSR: Decay*/
    PARAM_SUSTAIN,                                      /**< ADSR: Sustain*/
    PARAM_RELEASE,                                      /**< ADSR: Release*/
    PARAM_GAIN,                                         /**< Мастер-громкость*/
    PARAM_CRUSH,                                        /**< Lo-Fi: разрядность */
    PARAM_DOWNSAMPLE,                                   /**< Lo-Fi: прореживание */
    PARAM_CUTOFF,                                       /**< SVF-фильтр: частота среза*/
    PARAM_RESONANCE,                                    /**< SVF-фильтр: резонанс. */
    PARAM_PHASER_MIX,                                   /**< Фазер: сухой/влажный микс. */
    PARAM_PHASER_RATE,                                  /**< Фазер: частота LFO*/
    PARAM_PHASER_DEPTH,                                 /**< Фазер: глубина модуляции. */
    PARAM_PHASER_FEEDBACK,                              /**< Фазер: обратная связь. */
    PARAM_CHORUS_MIX,                                   /**< Хорус: сухой/влажный микс. */
    PARAM_CHORUS_RATE,                                  /**< Хорус: частота LFO*/
    PARAM_CHORUS_DEPTH,                                 /**< Хорус: глубина модуляции*/
    PARAM_REVERB_MIX,                                   /**< Реверб: сухой/влажный микс */
    PARAM_REVERB_SIZE,                                  /**< Реверб: размер комнаты. */
    PARAM_REVERB_DAMP,                                  /**< Реверб: демпфирование. */
    PARAM_OSC1_GAIN,                                    /**< Osc 1: громкость. */
    PARAM_OSC1_SEMI,                                    /**< Osc 1: транспонирование*/
    PARAM_OSC1_MORPH,                                   /**< Osc 1: форма волны. */
    PARAM_OSC2_GAIN,                                    /**< Osc 2: громкость. */
    PARAM_OSC2_SEMI,                                    /**< Osc 2: транспонирование */
    PARAM_OSC2_MORPH,                                   /**< Osc 2: форма волны. */
    PARAM_OSC3_GAIN,                                    /**< Osc 3: громкость. */
    PARAM_OSC3_SEMI,                                    /**< Osc 3: транспонирование */
    PARAM_OSC3_MORPH,                                   /**< Osc 3: форма волны. */
    PARAM_COUNT                                         /**< Общее количество параметров*/
} ParamId;

/**
 * @brief Полная структура экземпляра плагина.
 *
 * Объединяет CLAP-интерфейс, DSP-движок, GUI и менеджер памяти.
 */
typedef struct GranularSynth {
    clap_plugin_t plugin;                               /**< Базовый CLAP-интерфейс */
    const clap_host_t *host;                            /**< Указатель на хост*/
    double values[PARAM_COUNT];                         /**< Текущие значения всех параметров. */
    GranularEngine engine;                              /**< Гранулярный DSP-движок. */
    GuiState gui;                                       /**< Состояние GUI. */

    uint8_t memory_pool[PLUGIN_MEMORY_POOL_SIZE];       /**< Статический буфер памяти. */
    MemoryArena arena;                                  /**< Дескриптор арены памяти. */
} GranularSynth;

/** @brief Возвращает общее количество экспортируемых параметров. */
uint32_t params_get_count(void);

/** @brief Заполняет clap_param_info_t для параметра по индексу. */
bool params_get_info(uint32_t index, clap_param_info_t *info);

/** @brief Возвращает текущее значение параметра по его ID. */
bool params_get_value(const GranularSynth *synth, clap_id param_id, double *out_value);

/** @brief Форматирует значение параметра в текстовое представление. */
bool params_value_to_text(clap_id param_id, double value, char *out_buffer, uint32_t capacity);

/** @brief Применяет значение параметра к движку синтезатора. */
void params_apply_value(GranularSynth *synth, clap_id param_id, double value);

/** @brief Устанавливает все параметры в значения по умолчанию. */
void params_init_defaults(GranularSynth *synth);

/** @brief Сохраняет состояние (все параметры) в поток CLAP. */
bool params_state_save(const GranularSynth *synth, const clap_ostream_t *stream);

/** @brief Загружает состояние (все параметры) из потока CLAP. */
bool params_state_load(GranularSynth *synth, const clap_istream_t *stream);

#endif