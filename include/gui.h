#ifndef GUI_H
#define GUI_H

#include <clap/clap.h>
#include <clap/ext/gui.h>
#include <stdint.h>
#include <stdbool.h>

#define GUI_WIDTH 800
#define GUI_HEIGHT 500

typedef struct GuiState {
    uint32_t pixels[GUI_WIDTH * GUI_HEIGHT];
    void *native_view;
    void *parent_window;
    bool is_open;
} GuiState;

extern const clap_plugin_gui_t g_gui_extension;

void gui_init(GuiState *gui);
void gui_render_frame(GuiState *gui);

#endif