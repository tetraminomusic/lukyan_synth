/**
 * @file fx.h
 * @brief Модуль эффектов обработки звука.
 *
 * Содержит полную цепочку эффектов.
 */

#ifndef FX_H
#define FX_H

#define CHORUS_BUFFER_SIZE 96000                /**< Размер буфера задержки для хоруса. */
#define NUM_COMB 4                              /**< Количество гребёнчатых фильтров в ревербераторе. */
#define NUM_APF 2                               /**< Количество всепропускающих фильтров в ревербераторе. */
#define PHASER_STAGES 4                         /**< Количество ступеней Фазера. */

typedef struct {
    float crush_bits;                           /**< Разрядность для биткрашера*/
    float downsample;                           /**< Коэффициент прореживания*/
    float gain;                                 /**< Входное усиление мастер-секции*/
    float cutoff_hz;                            /**< Частота среза SVF-фильтра в герцах*/
    float resonance;                            /**< Резонанс SVF-фильтра*/
    
    float phaser_mix;                           /**< Сухой/влажный микс фазера*/
    float phaser_rate_hz;                       /**< Частота LFO фазера*/
    float phaser_depth;                         /**< Глубина свипирования фазера*/
    float phaser_feedback;                      /**< Обратная связь фазера*/

    float chorus_mix;                           /**< Сухой/влажный микс хоруса*/
    float chorus_rate_hz;                       /**< Частота LFO хоруса*/
    float chorus_depth_ms;                      /**< Глубина модуляции задержки хоруса*/

    float reverb_mix;                           /**< Сухой/влажный микс ревербератора*/
    float reverb_size;                          /**< Размер "комнаты"*/
    float reverb_damp;                          /**< Демпфирование высоких частот*/
} FxParams;

/**
 * @brief Внутреннее состояние всех эффектов (сохраняется между блоками).
 */
typedef struct {
    float ds_counter;                           /**< Счётчик для прореживания*/
    float held_sample_l;                        /**< Задержанный сэмпл L.*/
    float held_sample_r;                        /**< Задержанный сэмпл R.*/

    float ic1eq_l, ic2eq_l;                     /**< Интеграторы SVF для левого канала. */
    float ic1eq_r, ic2eq_r;                     /**< Интеграторы SVF для правого канала. */

    float phaser_x_l[PHASER_STAGES];            /**< Входы APF-ступеней L. */
    float phaser_y_l[PHASER_STAGES];            /**< Выходы APF-ступеней L. */
    float phaser_x_r[PHASER_STAGES];            /**< Входы APF-ступеней R. */
    float phaser_y_r[PHASER_STAGES];            /**< Выходы APF-ступеней R. */
    float phaser_lfo_phase;                     /**< Фаза LFO Фазера. */

    float chorus_buf_l[CHORUS_BUFFER_SIZE];     /**< Буфер задержки L. */
    float chorus_buf_r[CHORUS_BUFFER_SIZE];     /**< Буфер задержки R. */
    int write_pos;                              /**< Текущая позиция записи в буфер. */
    float chorus_lfo_phase;                     /**< Фаза LFO хоруса. */

    float comb_buf_l[NUM_COMB][2000];           /**< Буферы гребёнчатых фильтров L. */
    float comb_buf_r[NUM_COMB][2000];           /**< Буферы гребёнчатых фильтров R. */
    int comb_pos_l[NUM_COMB];                   /**< Позиции в гребёнчатых буферах L. */
    int comb_pos_r[NUM_COMB];                   /**< Позиции в гребёнчатых буферах R. */
    float comb_filter_l[NUM_COMB];              /**< Состояния демпфирования L. */
    float comb_filter_r[NUM_COMB];              /**< Состояния демпфирования R. */

    float apf_buf_l[NUM_APF][600];              /**< Буферы всепропускающих фильтров L. */
    float apf_buf_r[NUM_APF][600];              /**< Буферы всепропускающих фильтров R. */
    int apf_pos_l[NUM_APF];                     /**< Позиции в APF-буферах L. */
    int apf_pos_r[NUM_APF];                     /**< Позиции в APF-буферах R. */
} FxState;

/**
 * @brief Инициализирует состояние эффектов (обнуляет все буферы).
 * @param state Указатель на состояние эффектов.
 */
void fx_state_init(FxState *state);

/**
 * @brief Обрабатывает один стерео-сэмпл через всю цепочку эффектов.
 *
 * Порядок обработки: Gain → Lo-Fi → SVF → Phaser → Chorus → Reverb.
 *
 * @param state       Указатель на состояние эффектов.
 * @param params      Указатель на параметры эффектов.
 * @param out_l       Указатель на сэмпл левого канала.
 * @param out_r       Указатель на сэмпл правого канала.
 * @param sample_rate Частота дискретизации в Герцах.
 */
void fx_process(FxState *state, const FxParams *params, float *out_l, float *out_r, float sample_rate);

#endif