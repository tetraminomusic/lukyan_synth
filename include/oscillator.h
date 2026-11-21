/**
 * @file oscillator.h
 * @brief Волновые таблицы и морфинг осцилляторов.
 *
 * Содержит предрассчитанные таблицы четырёх базовых форм волн,
 * а также функцию их плавного морфинга с линейной интерполяцией.
 */

#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include <stdint.h>

#define NUM_WAVEFORMS 4          /**< Количество базовых форм волны. */
#define SAMPLE_BUFFER_SIZE 96000 /**< Размер волновой таблицы в сэмплах. */
#define BASE_ROOT_FREQ 110.0f    /**< Базовая частота для расчёта таблиц. */

/**
 * @brief Инициализирует волновые таблицы для заданной частоты дискретизации.
 *
 * Заполняет статический внутренний буфер sample_buffers четырьмя
 * предрассчитанными формами волн: Sine, Triangle, Sawtooth, Square.
 *
 * @param sample_rate Частота дискретизации, сообщённая хостом.
 */
void oscillator_init_buffers(double sample_rate);

/**
 * @brief Считывает значение из морфированной волновой таблицы.
 *
 * Выполняет линейную интерполяцию по позиции в таблице и по коэффициенту
 * морфинга
 *
 * @param pos   Дробная позиция считывания в таблице.
 * @param morph Коэффициент морфинга:
 * 
 * @return Значение волны в данной точке.
 */
float oscillator_read_morphed(float pos, float morph);

#endif