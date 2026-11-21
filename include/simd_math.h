/**
 * @file simd_math.h
 * @brief SIMD-оптимизированные функции обработки аудио.
 *
 * Предоставляет векторные (AVX2 на x86-64, NEON-совместимые на ARM)
 * функции для критически важных секций DSP. На x86-64 реализация
 * находится в simd_math_x86.S, на других платформах
 * используется Си-фоллбэк в simd_math_c.c.
 */

#ifndef SIMD_MATH_H
#define SIMD_MATH_H

#include <stdint.h>

/**
 * @brief Мгновенно обнуляет блок из 8 float для L и R каналов.
 *
 * На x86-64 использует `vxorps` для обнуления 256-битного регистра
 * и `vmovups` для записи в оба канала.
 *
 * @param out_l Буфер левого канала (8 float).
 * @param out_r Буфер правого канала (8 float).
 */
void simd_clear_block_stereo(float *out_l, float *out_r);

/**
 * @brief Векторное суммирование зерна в стерео-буферы (MAC-операция).
 *
 * Выполняет `out_l[i] += audio[i] * window[i] * gain_l` и аналогично
 * для правого канала, используя AVX2 + FMA (`vfmadd231ps`).
 *
 * @param out_l  Буфер накопления L.
 * @param out_r  Буфер накопления R.
 * @param audio  Буфер аудио-сэмплов зерна.
 * @param window Буфер значений окна Хэнна.
 * @param count  Количество сэмплов (кратно 8).
 * @param gain_l Громкость для L-канала.
 * @param gain_r Громкость для R-канала.
 */
void simd_grain_accumulate_stereo(float *out_l, float *out_r, const float *audio, const float *window, uint32_t count, float gain_l, float gain_r);

/**
 * @brief Векторная линейная интерполяция (LERP).
 *
 * Вычисляет `dest[i] = a[i] * (1 - frac) + b[i] * frac` для блока.
 *
 * @param dest  Буфер-назначение.
 * @param a     Первый буфер.
 * @param b     Второй буфер.
 * @param frac  Коэффициент интерполяции (0.0 - 1.0).
 * @param count Количество сэмплов (кратно 8).
 */
void simd_lerp_block(float *dest, const float *a, const float *b, float frac, uint32_t count);

/**
 * @brief Векторная обработка блока ADSR-огибающих.
 *
 * @param values        Массив текущих значений огибающих.
 * @param states        Массив состояний (IDLE/ATTACK/DECAY/SUSTAIN/RELEASE).
 * @param attack_step   Приращение за сэмпл на фазе атаки.
 * @param decay_step    Приращение за сэмпл на фазе спада.
 * @param sustain       Уровень удержания.
 * @param release_step  Приращение за сэмпл на фазе затухания.
 * @param count         Количество голосов.
 */
void simd_adsr_process_block(float *values, const uint32_t *states, float attack_step, float decay_step, float sustain, float release_step, uint32_t count);

/**
 * @brief Векторный стерео-фильтр SVF (State-Variable Filter).
 *
 * Оптимизирует вычисление коэффициентов SVF-фильтра для обоих каналов
 * одновременно с использованием FMA.
 *
 * @param samples_l Буфер левого канала (in-place обработка).
 * @param samples_r Буфер правого канала (in-place обработка).
 * @param count     Количество сэмплов для обработки.
 * @param cutoff_hz Частота среза в Герцах.
 * @param resonance Резонанс (Q-фактор).
 * @param sample_rate Частота дискретизации.
 */
void simd_svf_process_stereo(float *samples_l, float *samples_r, uint32_t count, float cutoff_hz, float resonance, float sample_rate);

#endif