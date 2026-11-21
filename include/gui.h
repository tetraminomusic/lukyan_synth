/**
 * @file gui.h
 * @brief Графический интерфейс плагина.
 *
 * Реализует рендеринг интерфейса в программный буфер пикселей
 * с последующим выводом в нативное окно хоста через
 * расширение CLAP_EXT_GUI.
 */

#ifndef GUI_H
#define GUI_H

#include <clap/clap.h>
#include <clap/ext/gui.h>
#include <stdint.h>
#include <stdbool.h>

#define GUI_WIDTH  960                          /**< Логическая ширина окна. */
#define GUI_HEIGHT 600                          /**< Логическая высота окна. */
#define GUI_SCALE  2                            /**< Коэффициент масштабирования (HiDPI). */
#define FB_WIDTH   (GUI_WIDTH * GUI_SCALE)      /**< Физическая ширина буфера пикселей. */
#define FB_HEIGHT  (GUI_HEIGHT * GUI_SCALE)     /**< Физическая высота буфера пикселей. */

/**
 * @brief Доступные вкладки интерфейса.
 */
typedef enum {
    TAB_OSC = 0,                                /**< Вкладка осцилляторов и гранулярного движка. */
    TAB_FX,                                     /**< Вкладка рэка эффектов. */
    TAB_PRESETS                                 /**< Вкладка менеджера пресетов. */
} GuiTab;

/**
 * @brief Доступные языки интерфейса.
 */
typedef enum {
    LANG_MEME_RU = 0,                           /**< Русский*/
    LANG_EN                                     /**< Английский*/
} GuiLang;

/**
 * @brief Полное состояние графического интерфейса.
 */
typedef struct GuiState {
    uint32_t pixels[FB_WIDTH * FB_HEIGHT];      /**< Буфер пикселей для рендеринга. */
    uint32_t bg_pixels[FB_WIDTH * FB_HEIGHT];   /**< Заранее подготовленный фон. */
    void *native_view;                          /**< Указатель на нативное представление (NSView/HWND). */
    void *parent_window;                        /**< Указатель на родительское окно хоста. */
    bool is_open;                               /**< Флаг видимости окна. */

    GuiTab current_tab;                         /**< Текущая активная вкладка. */
    GuiLang current_lang;                       /**< Текущий язык интерфейса. */
    int active_param_id;                        /**< ID параметра, который сейчас перетаскивается мышью (-1 если нет). */
    float drag_start_y;                         /**< Y-координата начала перетаскивания. */
    double drag_start_val;                      /**< Значение параметра в момент начала перетаскивания. */

    char current_preset_name[64];               /**< Имя текущего загруженного пресета. */
} GuiState;

struct GranularSynth;
typedef struct GranularSynth GranularSynth;

/**
 * @brief Расширение CLAP_GUI, экспортируемое плагином.
 */
extern const clap_plugin_gui_t g_gui_extension;

/**
 * @brief Инициализирует состояние GUI.
 * @param gui Указатель на состояние GUI.
 */
void gui_init(GuiState *gui);

/**
 * @brief Полностью перерисовывает кадр интерфейса и выводит его в нативное окно.
 * @param synth Указатель на экземпляр синтезатора.
 */
void gui_render_frame(GranularSynth *synth);

/**
 * @brief Обрабатывает событие нажатия левой кнопкой мыши.
 * @param synth Указатель на экземпляр синтезатора.
 * @param mx    X-координата курсора в логических пикселях.
 * @param my    Y-координата курсора в логических пикселях.
 */
void gui_handle_mouse_down(GranularSynth *synth, float mx, float my);

/**
 * @brief Обрабатывает событие перетаскивания мыши (с зажатой кнопкой).
 * @param synth Указатель на экземпляр синтезатора.
 * @param mx    X-координата курсора.
 * @param my    Y-координата курсора.
 */
void gui_handle_mouse_drag(GranularSynth *synth, float mx, float my);

/**
 * @brief Обрабатывает событие отпускания кнопки мыши.
 * @param synth Указатель на экземпляр синтезатора.
 */
void gui_handle_mouse_up(GranularSynth *synth);

#endif