#ifndef GUI_H
#define GUI_H

#include <clap/clap.h>
#include <clap/ext/gui.h>
#include <stdint.h>
#include <stdbool.h>

#define GUI_WIDTH 800
#define GUI_HEIGHT 500
#define GUI_SCALE 2
#define FB_WIDTH (GUI_WIDTH * GUI_SCALE)
#define FB_HEIGHT (GUI_HEIGHT * GUI_SCALE)

typedef enum {
    TAB_OSC = 0,
    TAB_FX
} GuiTab;

typedef enum {
    LANG_MEME_RU = 0,
    LANG_EN
} GuiLang;

typedef struct GuiState {
    uint32_t pixels[FB_WIDTH * FB_HEIGHT];
    uint32_t bg_pixels[FB_WIDTH * FB_HEIGHT];
    void *native_view;
    void *parent_window;
    bool is_open;

    GuiTab current_tab;
    GuiLang current_lang;
    int active_param_id;
    float drag_start_y;
    double drag_start_val;
} GuiState;

struct GranularSynth;
typedef struct GranularSynth GranularSynth;

extern const clap_plugin_gui_t g_gui_extension;

void gui_init(GuiState *gui);
void gui_render_frame(GranularSynth *synth);
void gui_handle_mouse_down(GranularSynth *synth, float mx, float my);
void gui_handle_mouse_drag(GranularSynth *synth, float mx, float my);
void gui_handle_mouse_up(GranularSynth *synth);

#endif