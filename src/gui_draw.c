#include "gui_draw.h"
#include "gui.h"
#include <string.h>
#include <math.h>

#define PI 3.14159265358979323846

static const uint8_t FONT_5X7[95][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, {0x00, 0x00, 0x5F, 0x00, 0x00}, {0x00, 0x07, 0x00, 0x07, 0x00},
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, {0x24, 0x2A, 0x7F, 0x2A, 0x12}, {0x23, 0x13, 0x08, 0x64, 0x62},
    {0x36, 0x49, 0x55, 0x22, 0x50}, {0x00, 0x05, 0x03, 0x00, 0x00}, {0x00, 0x1C, 0x22, 0x41, 0x00},
    {0x00, 0x41, 0x22, 0x1C, 0x00}, {0x14, 0x08, 0x3E, 0x08, 0x14}, {0x08, 0x08, 0x3E, 0x08, 0x08},
    {0x00, 0x50, 0x30, 0x00, 0x00}, {0x08, 0x08, 0x08, 0x08, 0x08}, {0x00, 0x60, 0x60, 0x00, 0x00},
    {0x20, 0x10, 0x08, 0x04, 0x02}, {0x3E, 0x51, 0x49, 0x45, 0x3E}, {0x00, 0x42, 0x7F, 0x40, 0x00},
    {0x42, 0x61, 0x51, 0x49, 0x46}, {0x21, 0x41, 0x45, 0x4B, 0x31}, {0x18, 0x14, 0x12, 0x7F, 0x10},
    {0x27, 0x45, 0x45, 0x45, 0x39}, {0x3C, 0x4A, 0x49, 0x49, 0x30}, {0x01, 0x71, 0x09, 0x05, 0x03},
    {0x36, 0x49, 0x49, 0x49, 0x36}, {0x06, 0x49, 0x49, 0x29, 0x1E}, {0x00, 0x36, 0x36, 0x00, 0x00},
    {0x00, 0x56, 0x36, 0x00, 0x00}, {0x08, 0x14, 0x22, 0x41, 0x00}, {0x14, 0x14, 0x14, 0x14, 0x14},
    {0x00, 0x41, 0x22, 0x14, 0x08}, {0x02, 0x01, 0x51, 0x09, 0x06}, {0x32, 0x49, 0x79, 0x41, 0x3E},
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, {0x7F, 0x49, 0x49, 0x49, 0x36}, {0x3E, 0x41, 0x41, 0x41, 0x22},
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, {0x7F, 0x49, 0x49, 0x49, 0x41}, {0x7F, 0x09, 0x09, 0x09, 0x01},
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, {0x7F, 0x08, 0x08, 0x08, 0x7F}, {0x00, 0x41, 0x7F, 0x41, 0x00},
    {0x20, 0x40, 0x41, 0x3F, 0x01}, {0x7F, 0x08, 0x14, 0x22, 0x41}, {0x7F, 0x40, 0x40, 0x40, 0x40},
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, {0x7F, 0x04, 0x08, 0x10, 0x7F}, {0x3E, 0x41, 0x41, 0x41, 0x3E},
    {0x7F, 0x09, 0x09, 0x09, 0x06}, {0x3E, 0x41, 0x51, 0x21, 0x5E}, {0x7F, 0x09, 0x19, 0x29, 0x46},
    {0x46, 0x49, 0x49, 0x49, 0x31}, {0x01, 0x01, 0x7F, 0x01, 0x01}, {0x3F, 0x40, 0x40, 0x40, 0x3F},
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, {0x3F, 0x40, 0x38, 0x40, 0x3F}, {0x63, 0x14, 0x08, 0x14, 0x63},
    {0x07, 0x08, 0x70, 0x08, 0x07}, {0x61, 0x51, 0x49, 0x45, 0x43}, {0x00, 0x7F, 0x41, 0x41, 0x00},
    {0x02, 0x04, 0x08, 0x10, 0x20}, {0x00, 0x41, 0x41, 0x7F, 0x00}, {0x04, 0x02, 0x01, 0x02, 0x04},
    {0x40, 0x40, 0x40, 0x40, 0x40}, {0x00, 0x01, 0x02, 0x04, 0x00}, {0x20, 0x54, 0x54, 0x54, 0x78},
    {0x7F, 0x48, 0x44, 0x44, 0x38}, {0x38, 0x44, 0x44, 0x44, 0x20}, {0x38, 0x44, 0x44, 0x48, 0x7F},
    {0x38, 0x54, 0x54, 0x54, 0x18}, {0x08, 0x7E, 0x09, 0x01, 0x02}, {0x0C, 0x52, 0x52, 0x52, 0x3E},
    {0x7F, 0x08, 0x04, 0x04, 0x78}, {0x00, 0x44, 0x7D, 0x40, 0x00}, {0x20, 0x40, 0x44, 0x3D, 0x00},
    {0x7F, 0x10, 0x28, 0x44, 0x00}, {0x00, 0x41, 0x7F, 0x40, 0x00}, {0x7C, 0x04, 0x18, 0x04, 0x78},
    {0x7C, 0x08, 0x04, 0x04, 0x78}, {0x38, 0x44, 0x44, 0x44, 0x38}, {0x7C, 0x14, 0x14, 0x14, 0x08},
    {0x08, 0x14, 0x14, 0x18, 0x7C}, {0x7C, 0x08, 0x04, 0x04, 0x08}, {0x48, 0x54, 0x54, 0x54, 0x20},
    {0x04, 0x3F, 0x44, 0x40, 0x20}, {0x3C, 0x40, 0x40, 0x20, 0x7C}, {0x1C, 0x20, 0x40, 0x20, 0x1C},
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, {0x44, 0x28, 0x10, 0x28, 0x44}, {0x0C, 0x50, 0x50, 0x50, 0x3C},
    {0x44, 0x64, 0x54, 0x4C, 0x44}, {0x00, 0x08, 0x36, 0x41, 0x00}, {0x00, 0x00, 0x7F, 0x00, 0x00},
    {0x00, 0x41, 0x36, 0x08, 0x00}, {0x08, 0x08, 0x2A, 0x1C, 0x08}
};

static const uint8_t GLYPH_B[5] = {0x7F, 0x49, 0x49, 0x49, 0x31};
static const uint8_t GLYPH_G[5] = {0x7F, 0x01, 0x01, 0x01, 0x01};
static const uint8_t GLYPH_D[5] = {0x60, 0x3F, 0x21, 0x3F, 0x60};
static const uint8_t GLYPH_ZH[5] = {0x49, 0x2A, 0x7F, 0x2A, 0x49};
static const uint8_t GLYPH_Z[5] = {0x22, 0x49, 0x49, 0x49, 0x36};
static const uint8_t GLYPH_I[5] = {0x7F, 0x10, 0x08, 0x04, 0x7F};
static const uint8_t GLYPH_YI[5] = {0x7F, 0x12, 0x09, 0x04, 0x7F};
static const uint8_t GLYPH_L[5] = {0x60, 0x1F, 0x01, 0x01, 0x7F};
static const uint8_t GLYPH_P[5] = {0x7F, 0x01, 0x01, 0x01, 0x7F};
static const uint8_t GLYPH_F[5] = {0x1C, 0x22, 0x7F, 0x22, 0x1C};
static const uint8_t GLYPH_TS[5] = {0x3F, 0x20, 0x20, 0x3F, 0xC0};
static const uint8_t GLYPH_CH[5] = {0x0F, 0x08, 0x08, 0x08, 0x7F};
static const uint8_t GLYPH_SH[5] = {0x7F, 0x40, 0x7F, 0x40, 0x7F};
static const uint8_t GLYPH_SHCH[5] = {0x3F, 0x20, 0x3F, 0x20, 0xE0};
static const uint8_t GLYPH_HARD[5] = {0x01, 0x7F, 0x48, 0x48, 0x30};
static const uint8_t GLYPH_Y[5] = {0x7F, 0x48, 0x30, 0x00, 0x7F};
static const uint8_t GLYPH_SOFT[5] = {0x7F, 0x48, 0x48, 0x48, 0x30};
static const uint8_t GLYPH_EE[5] = {0x22, 0x49, 0x49, 0x41, 0x3E};
static const uint8_t GLYPH_YU[5] = {0x7F, 0x08, 0x3E, 0x41, 0x3E};
static const uint8_t GLYPH_YA[5] = {0x46, 0x29, 0x19, 0x09, 0x7F};

void draw_copy_bg(uint32_t *dst, const uint32_t *src, int count) {
    memcpy(dst, src, count * sizeof(uint32_t));
}

static inline uint32_t blend_color(uint32_t bg, uint32_t fg, float alpha) {
    if (alpha <= 0.0f) return bg;
    if (alpha >= 1.0f) return fg;

    uint32_t bg_r = (bg >> 16) & 0xFF;
    uint32_t bg_g = (bg >> 8) & 0xFF;
    uint32_t bg_b = bg & 0xFF;

    uint32_t fg_r = (fg >> 16) & 0xFF;
    uint32_t fg_g = (fg >> 8) & 0xFF;
    uint32_t fg_b = fg & 0xFF;

    uint8_t out_r = (uint8_t)(bg_r + (fg_r - bg_r) * alpha);
    uint8_t out_g = (uint8_t)(bg_g + (fg_g - bg_g) * alpha);
    uint8_t out_b = (uint8_t)(bg_b + (fg_b - bg_b) * alpha);

    return (0xFF << 24) | (out_r << 16) | (out_g << 8) | out_b;
}

void draw_rect_panel(uint32_t *pixels, int x, int y, int w, int h, uint32_t bg_color, uint32_t border_color) {
    x *= GUI_SCALE;
    y *= GUI_SCALE;
    w *= GUI_SCALE;
    h *= GUI_SCALE;

    float a = (float)((bg_color >> 24) & 0xFF) / 255.0f;

    for (int py = y; py < y + h; ++py) {
        if (py < 0 || py >= FB_HEIGHT) continue;
        for (int px = x; px < x + w; ++px) {
            if (px < 0 || px >= FB_WIDTH) continue;

            if (px < x + 3 || px >= x + w - 3 || py < y + 3 || py >= y + h - 3) {
                pixels[py * FB_WIDTH + px] = border_color;
            } else {
                pixels[py * FB_WIDTH + px] = blend_color(pixels[py * FB_WIDTH + px], bg_color, a);
            }
        }
    }
}

static const uint8_t *get_glyph(uint32_t cp) {
    if (cp >= 32 && cp <= 126) return FONT_5X7[cp - 32];
    if (cp >= 0x0430 && cp <= 0x044F) cp -= 0x20;
    if (cp == 0x0451) cp = 0x0401;

    switch (cp) {
        case 0x0410: return FONT_5X7['A' - 32];
        case 0x0411: return GLYPH_B;
        case 0x0412: return FONT_5X7['B' - 32];
        case 0x0413: return GLYPH_G;
        case 0x0414: return GLYPH_D;
        case 0x0415: return FONT_5X7['E' - 32];
        case 0x0401: return FONT_5X7['E' - 32];
        case 0x0416: return GLYPH_ZH;
        case 0x0417: return GLYPH_Z;
        case 0x0418: return GLYPH_I;
        case 0x0419: return GLYPH_YI;
        case 0x041A: return FONT_5X7['K' - 32];
        case 0x041B: return GLYPH_L;
        case 0x041C: return FONT_5X7['M' - 32];
        case 0x041D: return FONT_5X7['H' - 32];
        case 0x041E: return FONT_5X7['O' - 32];
        case 0x041F: return GLYPH_P;
        case 0x0420: return FONT_5X7['P' - 32];
        case 0x0421: return FONT_5X7['C' - 32];
        case 0x0422: return FONT_5X7['T' - 32];
        case 0x0423: return FONT_5X7['Y' - 32];
        case 0x0424: return GLYPH_F;
        case 0x0425: return FONT_5X7['X' - 32];
        case 0x0426: return GLYPH_TS;
        case 0x0427: return GLYPH_CH;
        case 0x0428: return GLYPH_SH;
        case 0x0429: return GLYPH_SHCH;
        case 0x042A: return GLYPH_HARD;
        case 0x042B: return GLYPH_Y;
        case 0x042C: return GLYPH_SOFT;
        case 0x042D: return GLYPH_EE;
        case 0x042E: return GLYPH_YU;
        case 0x042F: return GLYPH_YA;
        default: return FONT_5X7['?' - 32];
    }
}

void draw_text(uint32_t *pixels, int x, int y, const char *str, uint32_t color) {
    if (!str) return;
    int cur_x = x * GUI_SCALE;
    int base_y = y * GUI_SCALE;

    const unsigned char *p = (const unsigned char *)str;

    while (*p) {
        uint32_t cp = 0;
        if (*p < 0x80) {
            cp = *p++;
        } else if ((*p & 0xE0) == 0xC0) {
            cp = ((*p & 0x1F) << 6) | (*(p + 1) & 0x3F);
            p += 2;
        } else if ((*p & 0xF0) == 0xE0) {
            cp = ((*p & 0x0F) << 12) | ((*(p + 1) & 0x3F) << 6) | (*(p + 2) & 0x3F);
            p += 3;
        } else {
            p++;
            continue;
        }

        const uint8_t *glyph = get_glyph(cp);

        // Тень
        for (int col = 0; col < 5; ++col) {
            uint8_t line = glyph[col];
            for (int row = 0; row < 7; ++row) {
                if (line & (1 << row)) {
                    for (int dy = 0; dy < GUI_SCALE; ++dy) {
                        for (int dx = 0; dx < GUI_SCALE; ++dx) {
                            int px = cur_x + col * GUI_SCALE + dx + 2;
                            int py = base_y + row * GUI_SCALE + dy + 2;
                            if (px >= 0 && px < FB_WIDTH && py >= 0 && py < FB_HEIGHT) {
                                pixels[py * FB_WIDTH + px] = blend_color(pixels[py * FB_WIDTH + px], 0xFF000000, 0.75f);
                            }
                        }
                    }
                }
            }
        }

        // Основные буквы
        for (int col = 0; col < 5; ++col) {
            uint8_t line = glyph[col];
            for (int row = 0; row < 7; ++row) {
                if (line & (1 << row)) {
                    for (int dy = 0; dy < GUI_SCALE; ++dy) {
                        for (int dx = 0; dx < GUI_SCALE; ++dx) {
                            int px = cur_x + col * GUI_SCALE + dx;
                            int py = base_y + row * GUI_SCALE + dy;
                            if (px >= 0 && px < FB_WIDTH && py >= 0 && py < FB_HEIGHT) {
                                pixels[py * FB_WIDTH + px] = color;
                            }
                        }
                    }
                }
            }
        }
        cur_x += 6 * GUI_SCALE;
    }
}

void draw_button(uint32_t *pixels, int x, int y, int w, int h, const char *label, bool is_selected) {
    uint32_t bg = is_selected ? 0xD0282C34 : 0x70181A1F;
    uint32_t border = is_selected ? 0xFF61AFEF : 0xFF3E4451;
    uint32_t txt = is_selected ? 0xFFFFFFFF : 0xFFABB2BF;

    draw_rect_panel(pixels, x, y, w, h, bg, border);

    int char_count = 0;
    const unsigned char *p = (const unsigned char *)label;
    while (*p) {
        if (*p < 0x80) p++;
        else if ((*p & 0xE0) == 0xC0) p += 2;
        else p++;
        char_count++;
    }

    int text_len = char_count * 6;
    int tx = x + (w - text_len) / 2;
    int ty = y + (h - 7) / 2;
    draw_text(pixels, tx, ty, label, txt);
}

// Расчет расстояния от точки до отрезка для субпиксельного рендеринга стрелки
static float dist_to_segment(float px, float py, float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float len_sq = dx * dx + dy * dy;
    if (len_sq == 0.0f) return sqrtf((px - x1) * (px - x1) + (py - y1) * (py - y1));
    float t = ((px - x1) * dx + (py - y1) * dy) / len_sq;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    float proj_x = x1 + t * dx;
    float proj_y = y1 + t * dy;
    return sqrtf((px - proj_x) * (px - proj_x) + (py - proj_y) * (py - proj_y));
}

void draw_knob(uint32_t *pixels, int cx, int cy, int radius, float norm_val, const char *name, const char *val_str, bool is_active) {
    if (norm_val < 0.0f) norm_val = 0.0f;
    if (norm_val > 1.0f) norm_val = 1.0f;

    float r_px = (float)(radius * GUI_SCALE);
    float cx_px = (float)(cx * GUI_SCALE);
    float cy_px = (float)(cy * GUI_SCALE);

    uint32_t ring_color = is_active ? 0xFF61AFEF : 0xFF6C7380;
    uint32_t fill_color = 0xF0181A1F;

    int r_bound = (int)(r_px + 2.0f);

    // Субпиксельный антиалиасинг окружности крутилки (Distance Field AA)
    for (int dy = -r_bound; dy <= r_bound; ++dy) {
        int py = (int)cy_px + dy;
        if (py < 0 || py >= FB_HEIGHT) continue;
        for (int dx = -r_bound; dx <= r_bound; ++dx) {
            int px = (int)cx_px + dx;
            if (px < 0 || px >= FB_WIDTH) continue;

            float dist = sqrtf((float)(dx * dx + dy * dy));

            // Внешний сглаженный край
            float outer_alpha = r_px - dist + 0.5f;
            if (outer_alpha <= 0.0f) continue;
            if (outer_alpha > 1.0f) outer_alpha = 1.0f;

            // Внутреннее заполнение
            float inner_dist = r_px - 5.0f;
            if (dist < inner_dist) {
                pixels[py * FB_WIDTH + px] = blend_color(pixels[py * FB_WIDTH + px], fill_color, outer_alpha);
            } else {
                float ring_alpha = outer_alpha;
                pixels[py * FB_WIDTH + px] = blend_color(pixels[py * FB_WIDTH + px], ring_color, ring_alpha);
            }
        }
    }

    float angle = (-135.0f + norm_val * 270.0f) * ((float)PI / 180.0f);
    float pointer_len = r_px - 6.0f;
    float end_x = cx_px + sinf(angle) * pointer_len;
    float end_y = cy_px - cosf(angle) * pointer_len;

    uint32_t needle_color = is_active ? 0xFF98C379 : 0xFFE5C07B;

    // Векторное субпиксельное сглаживание стрелки
    int min_x = (int)fminf(cx_px, end_x) - 4;
    int max_x = (int)fmaxf(cx_px, end_x) + 4;
    int min_y = (int)fminf(cy_px, end_y) - 4;
    int max_y = (int)fmaxf(cy_px, end_y) + 4;

    for (int py = min_y; py <= max_y; ++py) {
        if (py < 0 || py >= FB_HEIGHT) continue;
        for (int px = min_x; px <= max_x; ++px) {
            if (px < 0 || px >= FB_WIDTH) continue;

            float d = dist_to_segment((float)px, (float)py, cx_px, cy_px, end_x, end_y);
            float line_alpha = 1.8f - d;
            if (line_alpha <= 0.0f) continue;
            if (line_alpha > 1.0f) line_alpha = 1.0f;

            pixels[py * FB_WIDTH + px] = blend_color(pixels[py * FB_WIDTH + px], needle_color, line_alpha);
        }
    }

    int char_count = 0;
    const unsigned char *cp = (const unsigned char *)name;
    while (*cp) {
        if (*cp < 0x80) cp++;
        else if ((*cp & 0xE0) == 0xC0) cp += 2;
        else cp++;
        char_count++;
    }

    int name_len = char_count * 6;
    draw_text(pixels, cx - name_len / 2, cy + radius + 5, name, 0xFFFFFFFF);

    int val_len = (int)strlen(val_str) * 6;
    draw_text(pixels, cx - val_len / 2, cy + radius + 17, val_str, 0xFF98C379);
}