#ifndef GUI_H
#define GUI_H

#include <clap/clap.h>
#include <clap/ext/gui.h>
#include <stdint.h>
#include <stdbool.h>

#define GUI_WIDTH 800
#define GUI_HEIGHT 500

typedef enum {
    TAB_OSC = 0,
    TAB_FX
} GuiTab;

typedef struct GuiState {
    uint32_t pixels[GUI_WIDTH * GUI_HEIGHT];
    uint32_t bg_pixels[GUI_WIDTH * GUI_HEIGHT];
    void *native_view;
    void *parent_window;
    bool is_open;

    GuiTab current_tab;
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