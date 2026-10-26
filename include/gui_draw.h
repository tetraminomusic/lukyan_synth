#ifndef GUI_DRAW_H
#define GUI_DRAW_H

#include <stdint.h>
#include <stdbool.h>

void draw_copy_bg(uint32_t *dst, const uint32_t *src, int count);
void draw_rect_panel(uint32_t *pixels, int x, int y, int w, int h, uint32_t bg_color, uint32_t border_color);
void draw_text(uint32_t *pixels, int x, int y, const char *str, uint32_t color);
void draw_knob(uint32_t *pixels, int cx, int cy, int radius, float norm_val, const char *name, const char *val_str, bool is_active);
void draw_button(uint32_t *pixels, int x, int y, int w, int h, const char *label, bool is_selected);

#endif