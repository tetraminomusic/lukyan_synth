#include "gui.h"
#include "parameters.h"
#include <string.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "background_image.h"

#if defined(__APPLE__)
#include <CoreGraphics/CoreGraphics.h>
#include <objc/runtime.h>
#include <objc/message.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

void gui_init(GuiState *gui) {
    gui->native_view = NULL;
    gui->parent_window = NULL;
    gui->is_open = false;

    // Распаковываем запеченную фотку из памяти на чистом Си
    int img_w = 0, img_h = 0, channels = 0;
    
    // Подходит и для photo_png, и для photo_jpg (xxd сам называет массив по имени файла)
#if defined(photo_png) || defined(photo_png_len)
    unsigned char *data = stbi_load_from_memory(photo_png, photo_png_len, &img_w, &img_h, &channels, 4);
#elif defined(photo_jpg) || defined(photo_jpg_len)
    unsigned char *data = stbi_load_from_memory(photo_jpg, photo_jpg_len, &img_w, &img_h, &channels, 4);
#else
    unsigned char *data = NULL;
#endif

    if (data && img_w > 0 && img_h > 0) {
        for (int y = 0; y < GUI_HEIGHT; ++y) {
            for (int x = 0; x < GUI_WIDTH; ++x) {
                int src_x = (x * img_w) / GUI_WIDTH;
                int src_y = (y * img_h) / GUI_HEIGHT;
                int idx = (src_y * img_w + src_x) * 4;

                uint8_t r = data[idx];
                uint8_t g = data[idx + 1];
                uint8_t b = data[idx + 2];
                uint8_t a = data[idx + 3];

                // Накладываем легкое затемнение (на 30%), чтобы ручки и текст в будущем хорошо читались поверх фото
                r = (uint8_t)(r * 0.70f);
                g = (uint8_t)(g * 0.70f);
                b = (uint8_t)(b * 0.70f);

                gui->pixels[y * GUI_WIDTH + x] = (a << 24) | (r << 16) | (g << 8) | b;
            }
        }
        stbi_image_free(data);
    } else {
        // Запасной темный фон, если картинки нет
        for (int y = 0; y < GUI_HEIGHT; ++y) {
            for (int x = 0; x < GUI_WIDTH; ++x) {
                gui->pixels[y * GUI_WIDTH + x] = (0xFF << 24) | (22 << 16) | (22 << 8) | 26;
            }
        }
    }
}

void gui_render_frame(GuiState *gui) {
    if (!gui->is_open || !gui->native_view) return;

#if defined(__APPLE__)
    CGColorSpaceRef color_space = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, gui->pixels, GUI_WIDTH * GUI_HEIGHT * 4, NULL);
    CGImageRef image = CGImageCreate(GUI_WIDTH, GUI_HEIGHT, 8, 32, GUI_WIDTH * 4, color_space,
                                     kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Host,
                                     provider, NULL, false, kCGRenderingIntentDefault);

    id view = (id)gui->native_view;
    id layer = ((id (*)(id, SEL))objc_msgSend)(view, sel_registerName("layer"));
    if (layer) {
        ((void (*)(id, SEL, id))objc_msgSend)(layer, sel_registerName("setContents:"), (id)image);
    }

    CGImageRelease(image);
    CGDataProviderRelease(provider);
    CGColorSpaceRelease(color_space);
#endif
}

static bool gui_is_api_supported(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    (void)plugin;
    if (is_floating) return false;
#if defined(__APPLE__)
    return strcmp(api, CLAP_WINDOW_API_COCOA) == 0;
#elif defined(_WIN32)
    return strcmp(api, CLAP_WINDOW_API_WIN32) == 0;
#else
    return false;
#endif
}

static bool gui_get_preferred_api(const clap_plugin_t *plugin, const char **api, bool *is_floating) {
    (void)plugin;
    *is_floating = false;
#if defined(__APPLE__)
    *api = CLAP_WINDOW_API_COCOA;
    return true;
#elif defined(_WIN32)
    *api = CLAP_WINDOW_API_WIN32;
    return true;
#else
    return false;
#endif
}

static bool gui_create(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    (void)api; (void)is_floating;
    if (!plugin || !plugin->plugin_data) return false;
    GranularSynth *synth = (GranularSynth *)plugin->plugin_data;

    gui_init(&synth->gui);
    return true;
}

static void gui_destroy(const clap_plugin_t *plugin) {
    if (!plugin || !plugin->plugin_data) return;
    GranularSynth *synth = (GranularSynth *)plugin->plugin_data;
    synth->gui.is_open = false;
    synth->gui.native_view = NULL;
    synth->gui.parent_window = NULL;
}

static bool gui_set_scale(const clap_plugin_t *plugin, double scale) { (void)plugin; (void)scale; return true; }

static bool gui_get_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    (void)plugin;
    *width = GUI_WIDTH;
    *height = GUI_HEIGHT;
    return true;
}

static bool gui_can_resize(const clap_plugin_t *plugin) { (void)plugin; return false; }
static bool gui_get_resize_hints(const clap_plugin_t *plugin, clap_gui_resize_hints_t *hints) { (void)plugin; (void)hints; return false; }
static bool gui_adjust_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    (void)plugin;
    *width = GUI_WIDTH;
    *height = GUI_HEIGHT;
    return true;
}

static bool gui_set_size(const clap_plugin_t *plugin, uint32_t width, uint32_t height) {
    (void)plugin;
    return (width == GUI_WIDTH && height == GUI_HEIGHT);
}

static bool gui_set_parent(const clap_plugin_t *plugin, const clap_window_t *window) {
    if (!plugin || !plugin->plugin_data || !window) return false;
    GranularSynth *synth = (GranularSynth *)plugin->plugin_data;

#if defined(__APPLE__)
    id parent_view = (id)window->cocoa;
    if (!parent_view) return false;

    id view_class = (id)objc_getClass("NSView");
    id my_view = ((id (*)(id, SEL))objc_msgSend)(view_class, sel_registerName("alloc"));
    CGRect frame = CGRectMake(0, 0, GUI_WIDTH, GUI_HEIGHT);
    my_view = ((id (*)(id, SEL, CGRect))objc_msgSend)(my_view, sel_registerName("initWithFrame:"), frame);

    ((void (*)(id, SEL, BOOL))objc_msgSend)(my_view, sel_registerName("setWantsLayer:"), (BOOL)1);
    ((void (*)(id, SEL, id))objc_msgSend)(parent_view, sel_registerName("addSubview:"), my_view);

    synth->gui.native_view = (void *)my_view;
    synth->gui.parent_window = (void *)parent_view;
    synth->gui.is_open = true;

    gui_render_frame(&synth->gui);
    return true;
#elif defined(_WIN32)
    synth->gui.parent_window = window->win32;
    synth->gui.native_view = window->win32;
    synth->gui.is_open = true;
    return true;
#else
    return false;
#endif
}

static bool gui_set_transient(const clap_plugin_t *plugin, const clap_window_t *window) { (void)plugin; (void)window; return true; }
static void gui_suggest_title(const clap_plugin_t *plugin, const char *title) { (void)plugin; (void)title; }

static bool gui_show(const clap_plugin_t *plugin) {
    if (!plugin || !plugin->plugin_data) return false;
    GranularSynth *synth = (GranularSynth *)plugin->plugin_data;
    synth->gui.is_open = true;
    gui_render_frame(&synth->gui);
    return true;
}

static bool gui_hide(const clap_plugin_t *plugin) {
    if (!plugin || !plugin->plugin_data) return false;
    GranularSynth *synth = (GranularSynth *)plugin->plugin_data;
    synth->gui.is_open = false;
    return true;
}

const clap_plugin_gui_t g_gui_extension = {
    .is_api_supported = gui_is_api_supported,
    .get_preferred_api = gui_get_preferred_api,
    .create = gui_create,
    .destroy = gui_destroy,
    .set_scale = gui_set_scale,
    .get_size = gui_get_size,
    .can_resize = gui_can_resize,
    .get_resize_hints = gui_get_resize_hints,
    .adjust_size = gui_adjust_size,
    .set_size = gui_set_size,
    .set_parent = gui_set_parent,
    .set_transient = gui_set_transient,
    .suggest_title = gui_suggest_title,
    .show = gui_show,
    .hide = gui_hide,
};