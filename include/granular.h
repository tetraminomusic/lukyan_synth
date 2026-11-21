/**
 * @file granular.h
 * @brief Гранулярный аудиодвижок — ядро синтезатора.
 *
 * Управляет пулом зёрен, полифоническими голосами, ADSR-огибающими,
 * генерацией 3 осцилляторов с морфингом волновых таблиц и вызовом
 * цепочки эффектов.
 */

#ifndef GRANULAR_H
#define GRANULAR_H

#include <stdbool.h>
#include <stdint.h>
#include "adsr.h"
#include "fx.h"
#include "oscillator.h"

#define MAX_VOICES 16          /**< Максимальное число полифонических голосов. */
#define MAX_GRAINS 256         /**< Размер статического пула зёрен. */
#define HANN_LUT_SIZE 2048     /**< Размер таблицы окна Хэнна (Look-Up Table). */
#define NUM_OSCS 3             /**< Количество осцилляторов на голос. */

/**
 * @brief Описание одного активного зерна гранулярного синтеза.
 */
typedef struct {
    bool active;                /**< Активно ли зерно в данный момент. */
    float pos[NUM_OSCS];        /**< Текущая позиция считывания для каждого осциллятора. */
    float speed[NUM_OSCS];      /**< Скорость считывания для каждого осциллятора. */
    float length;               /**< Общая длина зерна в сэмплах. */
    float progress;             /**< Текущий прогресс зерна (0..length). */
    float pan;                  /**< Панорама зерна (0.0 = L, 1.0 = R). */
    float amp;                  /**< Амплитуда зерна (из ADSR). */
} Grain;

/**
 * @brief Описание полифонического голоса.
 */
typedef struct {
    bool active;                /**< Активен ли голос. */
    int32_t key;                /**< MIDI-номер ноты. */
    float frequency;            /**< Частота ноты в Герцах. */
    float spawn_timer;          /**< Счётчик до следующего рождения зерна. */
    AdsrVoice adsr;             /**< Состояние ADSR-огибающей голоса. */
} Voice;

/**
 * @brief Полное состояние гранулярного движка.
 */
typedef struct {
    double sample_rate;               /**< Частота дискретизации в Герцах. */
    float playhead;                   /**< Текущая позиция считывания в буфере. */
    uint32_t next_voice_rr;           /**< Индекс для round-robin замены голосов. */
    uint32_t idle_tail_frames;        /**< Счётчик для детектора тишины (Auto-Sleep). */

    float grain_size_ms;              /**< Длина зерна в миллисекундах. */
    float density;                    /**< Плотность рождения зёрен (зёрен/сек). */
    float spray;                      /**< Случайный разброс позиции старта зерна. */
    float pitch_bend_semitones;       /**< Текущий pitch bend в полутонах. */

    float osc_gain[NUM_OSCS];         /**< Громкость каждого осциллятора. */
    float osc_semi[NUM_OSCS];         /**< Транспонирование в полутонах. */
    float osc_morph[NUM_OSCS];        /**< Морфинг формы волны (0.0 - 3.0). */

    AdsrParams adsr_params;           /**< Параметры ADSR-огибающей. */
    FxParams fx_params;               /**< Параметры эффектов. */
    FxState fx_state;                 /**< Состояние эффектов. */

    float hann_lut[HANN_LUT_SIZE];    /**< Таблица значений окна Хэнна. */
    Grain grains[MAX_GRAINS];         /**< Статический пул зёрен. */
    Voice voices[MAX_VOICES];         /**< Пул полифонических голосов. */
} GranularEngine;

/**
 * @brief Инициализирует движок.
 * @param engine      Указатель на движок.
 * @param sample_rate Частота дискретизации, сообщённая хостом.
 */
void granular_init(GranularEngine *engine, double sample_rate);

/**
 * @brief Обрабатывает событие Note On.
 * @param engine    Указатель на движок.
 * @param key       MIDI-номер ноты.
 * @param frequency Частота ноты в Герцах.
 */
void granular_note_on(GranularEngine *engine, int32_t key, float frequency);

/**
 * @brief Обрабатывает событие Note Off.
 * @param engine Указатель на движок.
 * @param key    MIDI-номер ноты.
 */
void granular_note_off(GranularEngine *engine, int32_t key);

/**
 * @brief Полностью сбрасывает состояние движка (например, при reset()).
 * @param engine Указатель на движок.
 */
void granular_reset(GranularEngine *engine);


/** @brief Устанавливает длину зерна в мс. */
void granular_set_grain_size(GranularEngine *engine, float size_ms);
/** @brief Устанавливает плотность зёрен. */
void granular_set_density(GranularEngine *engine, float density);
/** @brief Устанавливает коэффициент разброса зерна. */
void granular_set_spray(GranularEngine *engine, float spray);
/** @brief Устанавливает значение pitch bend в полутонах. */
void granular_set_pitch_bend(GranularEngine *engine, float semitones);
/** @brief Устанавливает громкость осциллятора. */
void granular_set_osc_gain(GranularEngine *engine, uint32_t osc_idx, float gain);
/** @brief Устанавливает транспонирование осциллятора. */
void granular_set_osc_semi(GranularEngine *engine, uint32_t osc_idx, float semi);
/** @brief Устанавливает морфинг формы волны осциллятора. */
void granular_set_osc_morph(GranularEngine *engine, uint32_t osc_idx, float morph);
/** @brief Устанавливает время атаки ADSR. */
void granular_set_attack(GranularEngine *engine, float attack_ms);
/** @brief Устанавливает время спада ADSR. */
void granular_set_decay(GranularEngine *engine, float decay_ms);
/** @brief Устанавливает уровень удержания ADSR. */
void granular_set_sustain(GranularEngine *engine, float sustain);
/** @brief Устанавливает время затухания ADSR. */
void granular_set_release(GranularEngine *engine, float release_ms);

/**
 * @brief Устаревшая функция рендеринга одного сэмпла (не используется).
 * @deprecated Используйте granular_render_block().
 */
void granular_render_sample(GranularEngine *engine, float *out_l, float *out_r);

/**
 * @brief Рендерит блок аудио с использованием SIMD-оптимизаций.
 *
 * Основная функция обработки. Возвращает false, если сигнал полностью
 * затих и плагин может "уснуть" (CLAP_PROCESS_SLEEP).
 *
 * @param engine Указатель на движок.
 * @param out_l  Буфер левого канала (может быть NULL).
 * @param out_r  Буфер правого канала (может быть NULL).
 * @param frames Количество фреймов для рендеринга.
 * @return true, если сигнал активен; false, если можно уснуть.
 */
bool granular_render_block(GranularEngine *engine, float *out_l, float *out_r, uint32_t frames);

#endif