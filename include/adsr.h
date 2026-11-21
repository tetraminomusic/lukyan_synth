/**
 * @file adsr.h
 * @brief ADSR-огибающая для звукового сигнала синтезатора
 * 
 * Реализует классический вариант 4 ступенчатой обработки звука через Attack, Decay, Sustain и Release.
 */


#ifndef ADSR_H
#define ADSR_H

#include <stdbool.h>

/**
 * @brief Состояние конечного автомата ADSR-огибающей
 */
typedef enum {
    ADSR_IDLE = 0,  /**< Голос неактивен, огибающая на нуле. */
    ADSR_ATTACK,    /**< Фаза нарастания: значение растёт от 0 до 1. */
    ADSR_DECAY,     /**< Фаза спада: значение падает от 1 до sustain. */
    ADSR_SUSTAIN,   /**< Фаза удержания: значение стабильно на уровне sustain. */
    ADSR_RELEASE    /**< Фаза затухания: значение падает от текущего до 0. */
} AdsrState;

/**
 * @brief Параметры ADSR-огибающей
 */
typedef struct {
    float attack_ms;   /**< Время атаки в миллисекундах (1.0 - 2000.0). */
    float decay_ms;    /**< Время спада в миллисекундах (10.0 - 2000.0). */
    float sustain;     /**< Уровень удержания, нормированный (0.0 - 1.0). */
    float release_ms;  /**< Время затухания в миллисекундах (10.0 - 3000.0). */
} AdsrParams;


/**
 * @brief Состояние ADSR-огибающей для одного голоса.
 */
typedef struct {
    AdsrState state;  /**< Текущее состояние конечного автомата. */
    float value;      /**< Текущее значение огибающей (0.0 - 1.0). */
} AdsrVoice;


/**
 * @brief Инициализирует состояние огибающей голоса.
 * @param voice Указатель на состояние огибающей.
 */
void adsr_voice_init(AdsrVoice *voice);

/**
 * @brief Запускает огибающую.
 *
 * Переводит конечный автомат в состояние ADSR_ATTACK.
 *
 * @param voice Указатель на состояние огибающей.
 */
void adsr_voice_gate_on(AdsrVoice *voice);

/**
 * @brief Останавливает огибающую.
 *
 * Переводит конечный автомат в состояние ADSR_RELEASE.
 *
 * @param voice Указатель на состояние огибающей.
 */
void adsr_voice_gate_off(AdsrVoice *voice);

/**
 * @brief Продвигает огибающую на один сэмпл.
 *
 * Вызывается из аудиопотока для каждого сэмпла и обновляет
 * значение огибающей в соответствии с текущим состоянием.
 *
 * @param voice   Указатель на состояние огибающей.
 * @param params  Указатель на параметры огибающей.
 * @param sr      Частота дискретизации в Герцах.
 */
void adsr_voice_process(AdsrVoice *voice, const AdsrParams *params, float sr);

#endif
