#include "gui.h"
#include "gui_draw.h"
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

typedef struct {
    int id;
    int cx;
    int cy;
    int r;
    const char *label;
} KnobLayout;

void gui_init(GuiState *gui) {
    gui->native_view = NULL;
    gui->parent_window = NULL;
    gui->is_open = false;
    gui->current_tab = TAB_OSC;
    gui->active_param_id = -1;

    int img_w = 0, img_h = 0, channels = 0;
    unsigned char *data = stbi_load_from_memory(photo_png, photo_png_len, &img_w, &img_h, &channels, 4);
    (void)channels;

    if (data && img_w > 0 && img_h > 0) {
        for (int y = 0; y < FB_HEIGHT; ++y) {
            for (int x = 0; x < FB_WIDTH; ++x) {
                int src_x = (x * img_w) / FB_WIDTH;
                int src_y = (y * img_h) / FB_HEIGHT;
                int idx = (src_y * img_w + src_x) * 4;

                uint8_t r = (uint8_t)(data[idx] * 0.45f);
                uint8_t g = (uint8_t)(data[idx + 1] * 0.45f);
                uint8_t b = (uint8_t)(data[idx + 2] * 0.45f);

                gui->bg_pixels[y * FB_WIDTH + x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
            }
        }
        stbi_image_free(data);
    } else {
        for (int i = 0; i < FB_WIDTH * FB_HEIGHT; ++i) {
            gui->bg_pixels[i] = (0xFF << 24) | (20 << 16) | (22 << 8) | 26;
        }
    }
}

static void draw_tab_osc(GranularSynth *synth) {
    uint32_t *px = synth->gui.pixels;

    draw_rect_panel(px, 15, 60, 245, 230, 0xA01E222A, 0xFF3E4451);
    draw_text(px, 25, 70, "OSCILLATOR 1", 0xFF61AFEF);

    draw_rect_panel(px, 275, 60, 245, 230, 0xA01E222A, 0xFF3E4451);
    draw_text(px, 285, 70, "OSCILLATOR 2", 0xFF98C379);

    draw_rect_panel(px, 535, 60, 250, 230, 0xA01E222A, 0xFF3E4451);
    draw_text(px, 545, 70, "OSCILLATOR 3", 0xFFE5C07B);

    draw_rect_panel(px, 15, 305, 380, 180, 0xA01E222A, 0xFF3E4451);
    draw_text(px, 25, 315, "GRANULAR CLOUD", 0xFFC678DD);

    draw_rect_panel(px, 410, 305, 375, 180, 0xA01E222A, 0xFF3E4451);
    draw_text(px, 420, 315, "ADSR ENVELOPE & MASTER", 0xFFE06C75);

    static const KnobLayout KNOBS_OSC[] = {
        { PARAM_OSC1_GAIN, 65, 140, 20, "Gain" }, { PARAM_OSC1_SEMI, 137, 140, 20, "Semi" }, { PARAM_OSC1_MORPH, 210, 140, 20, "Morph" },
        { PARAM_OSC2_GAIN, 325, 140, 20, "Gain" }, { PARAM_OSC2_SEMI, 397, 140, 20, "Semi" }, { PARAM_OSC2_MORPH, 470, 140, 20, "Morph" },
        { PARAM_OSC3_GAIN, 585, 140, 20, "Gain" }, { PARAM_OSC3_SEMI, 660, 140, 20, "Semi" }, { PARAM_OSC3_MORPH, 735, 140, 20, "Morph" },
        { PARAM_GRAIN_SIZE, 75, 385, 22, "Size" }, { PARAM_DENSITY, 195, 385, 22, "Density" }, { PARAM_SPRAY, 315, 385, 22, "Spray" },
        { PARAM_ATTACK, 460, 370, 18, "Attack" }, { PARAM_DECAY, 525, 370, 18, "Decay" }, { PARAM_SUSTAIN, 590, 370, 18, "Sustain" },
        { PARAM_RELEASE, 655, 370, 18, "Release" }, { PARAM_GAIN, 730, 370, 18, "Master" }
    };

    for (size_t i = 0; i < sizeof(KNOBS_OSC) / sizeof(KNOBS_OSC[0]); ++i) {
        int id = KNOBS_OSC[i].id;
        clap_param_info_t info;
        params_get_info(id, &info);

        double val = 0.0;
        params_get_value(synth, id, &val);

        float norm = (float)((val - info.min_value) / (info.max_value - info.min_value));
        char val_str[32];
        params_value_to_text(id, val, val_str, sizeof(val_str));

        draw_knob(px, KNOBS_OSC[i].cx, KNOBS_OSC[i].cy, KNOBS_OSC[i].r, norm, KNOBS_OSC[i].label, val_str, synth->gui.active_param_id == id);
    }
}

static void draw_tab_fx(GranularSynth *synth) {
    uint32_t *px = synth->gui.pixels;

    draw_rect_panel(px, 15, 60, 245, 200, 0xA01E222A, 0xFF3E4451);
    draw_text(px, 25, 70, "FILTER (SVF)", 0xFF61AFEF);

    draw_rect_panel(px, 275, 60, 245, 200, 0xA01E222A, 0xFF3E4451);
    draw_text(px, 285, 70, "LO-FI CRUNCH", 0xFFE06C75);

    draw_rect_panel(px, 535, 60, 250, 200, 0xA01E222A, 0xFF3E4451);
    draw_text(px, 545, 70, "PHASER (4-STAGE)", 0xFFE5C07B);

    draw_rect_panel(px, 15, 280, 380, 205, 0xA01E222A, 0xFF3E4451);
    draw_text(px, 25, 290, "STEREO CHORUS", 0xFF98C379);

    draw_rect_panel(px, 410, 280, 375, 205, 0xA01E222A, 0xFF3E4451);
    draw_text(px, 420, 290, "SCHROEDER REVERB", 0xFFC678DD);

    static const KnobLayout KNOBS_FX[] = {
        { PARAM_CUTOFF, 80, 145, 22, "Cutoff" }, { PARAM_RESONANCE, 190, 145, 22, "Res" },
        { PARAM_CRUSH, 340, 145, 22, "Bits" }, { PARAM_DOWNSAMPLE, 450, 145, 22, "Rate" },
        
        { PARAM_PHASER_MIX, 568, 135, 17, "Mix" },
        { PARAM_PHASER_RATE, 626, 135, 17, "Rate" },
        { PARAM_PHASER_DEPTH, 684, 135, 17, "Depth" },
        { PARAM_PHASER_FEEDBACK, 742, 135, 17, "Fbk" },

        { PARAM_CHORUS_MIX, 80, 370, 22, "Mix" }, { PARAM_CHORUS_RATE, 195, 370, 22, "Rate" }, { PARAM_CHORUS_DEPTH, 310, 370, 22, "Depth" },
        { PARAM_REVERB_MIX, 480, 370, 22, "Mix" }, { PARAM_REVERB_SIZE, 595, 370, 22, "Size" }, { PARAM_REVERB_DAMP, 710, 370, 22, "Damp" }
    };

    for (size_t i = 0; i < sizeof(KNOBS_FX) / sizeof(KNOBS_FX[0]); ++i) {
        int id = KNOBS_FX[i].id;
        clap_param_info_t info;
        params_get_info(id, &info);

        double val = 0.0;
        params_get_value(synth, id, &val);

        float norm = (float)((val - info.min_value) / (info.max_value - info.min_value));
        char val_str[32];
        params_value_to_text(id, val, val_str, sizeof(val_str));

        draw_knob(px, KNOBS_FX[i].cx, KNOBS_FX[i].cy, KNOBS_FX[i].r, norm, KNOBS_FX[i].label, val_str, synth->gui.active_param_id == id);
    }
}

void gui_render_frame(GranularSynth *synth) {
    if (!synth || !synth->gui.is_open || !synth->gui.native_view) return;

    draw_copy_bg(synth->gui.pixels, synth->gui.bg_pixels, FB_WIDTH * FB_HEIGHT);

    draw_rect_panel(synth->gui.pixels, 0, 0, GUI_WIDTH, 50, 0xD0181A1F, 0xFF282C34);
    draw_text(synth->gui.pixels, 20, 20, "LUKYAN SYNTH  //  POLYPHONIC GRANULAR", 0xFFE5C07B);
    draw_text(synth->gui.pixels, 340, 20, "by tetramino", 0xFF5C6370);

    draw_button(synth->gui.pixels, 520, 12, 125, 28, "1. OSC & CORE", synth->gui.current_tab == TAB_OSC);
    draw_button(synth->gui.pixels, 655, 12, 125, 28, "2. FX RACK", synth->gui.current_tab == TAB_FX);

    if (synth->gui.current_tab == TAB_OSC) {
        draw_tab_osc(synth);
    } else {
        draw_tab_fx(synth);
    }

#if defined(__APPLE__)
    CGColorSpaceRef color_space = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, synth->gui.pixels, FB_WIDTH * FB_HEIGHT * 4, NULL);
    CGImageRef image = CGImageCreate(FB_WIDTH, FB_HEIGHT, 8, 32, FB_WIDTH * 4, color_space,
                                     kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Host,
                                     provider, NULL, false, kCGRenderingIntentDefault);

    id view = (id)synth->gui.native_view;
    id layer = ((id (*)(id, SEL))objc_msgSend)(view, sel_registerName("layer"));
    if (layer) {
        ((void (*)(id, SEL, CGFloat))objc_msgSend)(layer, sel_registerName("setContentsScale:"), (CGFloat)GUI_SCALE);
        ((void (*)(id, SEL, id))objc_msgSend)(layer, sel_registerName("setContents:"), (id)image);
    }

    CGImageRelease(image);
    CGDataProviderRelease(provider);
    CGColorSpaceRelease(color_space);
#endif
}

void gui_handle_mouse_down(GranularSynth *synth, float mx, float my) {
    if (my >= 12 && my <= 40) {
        if (mx >= 520 && mx <= 645) {
            synth->gui.current_tab = TAB_OSC;
            gui_render_frame(synth);
            return;
        }
        if (mx >= 655 && mx <= 780) {
            synth->gui.current_tab = TAB_FX;
            gui_render_frame(synth);
            return;
        }
    }

    int clicked_param = -1;

    if (synth->gui.current_tab == TAB_OSC) {
        static const KnobLayout KNOBS_OSC[] = {
            { PARAM_OSC1_GAIN, 65, 140, 20, "Gain" }, { PARAM_OSC1_SEMI, 137, 140, 20, "Semi" }, { PARAM_OSC1_MORPH, 210, 140, 20, "Morph" },
            { PARAM_OSC2_GAIN, 325, 140, 20, "Gain" }, { PARAM_OSC2_SEMI, 397, 140, 20, "Semi" }, { PARAM_OSC2_MORPH, 470, 140, 20, "Morph" },
            { PARAM_OSC3_GAIN, 585, 140, 20, "Gain" }, { PARAM_OSC3_SEMI, 660, 140, 20, "Semi" }, { PARAM_OSC3_MORPH, 735, 140, 20, "Morph" },
            { PARAM_GRAIN_SIZE, 75, 385, 22, "Size" }, { PARAM_DENSITY, 195, 385, 22, "Density" }, { PARAM_SPRAY, 315, 385, 22, "Spray" },
            { PARAM_ATTACK, 460, 370, 18, "Attack" }, { PARAM_DECAY, 525, 370, 18, "Decay" }, { PARAM_SUSTAIN, 590, 370, 18, "Sustain" },
            { PARAM_RELEASE, 655, 370, 18, "Release" }, { PARAM_GAIN, 730, 370, 18, "Master" }
        };
        for (size_t i = 0; i < sizeof(KNOBS_OSC) / sizeof(KNOBS_OSC[0]); ++i) {
            float dx = mx - (float)KNOBS_OSC[i].cx;
            float dy = my - (float)KNOBS_OSC[i].cy;
            if (dx * dx + dy * dy <= (float)(KNOBS_OSC[i].r * KNOBS_OSC[i].r * 2)) {
                clicked_param = KNOBS_OSC[i].id;
                break;
            }
        }
    } else {
        static const KnobLayout KNOBS_FX[] = {
            { PARAM_CUTOFF, 80, 145, 22, "Cutoff" }, { PARAM_RESONANCE, 190, 145, 22, "Res" },
            { PARAM_CRUSH, 340, 145, 22, "Bits" }, { PARAM_DOWNSAMPLE, 450, 145, 22, "Rate" },
            { PARAM_PHASER_MIX, 568, 135, 17, "Mix" },
            { PARAM_PHASER_RATE, 626, 135, 17, "Rate" },
            { PARAM_PHASER_DEPTH, 684, 135, 17, "Depth" },
            { PARAM_PHASER_FEEDBACK, 742, 135, 17, "Fbk" },
            { PARAM_CHORUS_MIX, 80, 370, 22, "Mix" }, { PARAM_CHORUS_RATE, 195, 370, 22, "Rate" }, { PARAM_CHORUS_DEPTH, 310, 370, 22, "Depth" },
            { PARAM_REVERB_MIX, 480, 370, 22, "Mix" }, { PARAM_REVERB_SIZE, 595, 370, 22, "Size" }, { PARAM_REVERB_DAMP, 710, 370, 22, "Damp" }
        };
        for (size_t i = 0; i < sizeof(KNOBS_FX) / sizeof(KNOBS_FX[0]); ++i) {
            float dx = mx - (float)KNOBS_FX[i].cx;
            float dy = my - (float)KNOBS_FX[i].cy;
            if (dx * dx + dy * dy <= (float)(KNOBS_FX[i].r * KNOBS_FX[i].r * 2)) {
                clicked_param = KNOBS_FX[i].id;
                break;
            }
        }
    }

    if (clicked_param != -1) {
        synth->gui.active_param_id = clicked_param;
        synth->gui.drag_start_y = my;
        params_get_value(synth, clicked_param, &synth->gui.drag_start_val);
        gui_render_frame(synth);
    }
}

void gui_handle_mouse_drag(GranularSynth *synth, float mx, float my) {
    (void)mx;
    if (synth->gui.active_param_id == -1) return;

    clap_param_info_t info;
    params_get_info(synth->gui.active_param_id, &info);

    float delta_y = synth->gui.drag_start_y - my;
    double range = info.max_value - info.min_value;
    double new_val = synth->gui.drag_start_val + (double)delta_y * (range / 150.0);

    if (new_val < info.min_value) new_val = info.min_value;
    if (new_val > info.max_value) new_val = info.max_value;

    if (info.flags & CLAP_PARAM_IS_STEPPED) {
        new_val = round(new_val);
    }

    params_apply_value(synth, synth->gui.active_param_id, new_val);
    gui_render_frame(synth);
}

void gui_handle_mouse_up(GranularSynth *synth) {
    if (synth->gui.active_param_id != -1) {
        synth->gui.active_param_id = -1;
        gui_render_frame(synth);
    }
}

#if defined(__APPLE__)
static void mac_mouse_down(id self, SEL _cmd, id event) {
    (void)_cmd;
    GranularSynth *synth = (GranularSynth *)objc_getAssociatedObject(self, "synth");
    if (!synth) return;

    CGPoint win_p = ((CGPoint (*)(id, SEL))objc_msgSend)(event, sel_registerName("locationInWindow"));
    CGPoint p = ((CGPoint (*)(id, SEL, CGPoint, id))objc_msgSend)(self, sel_registerName("convertPoint:fromView:"), win_p, (id)NULL);
    gui_handle_mouse_down(synth, (float)p.x, (float)(GUI_HEIGHT - p.y));
}

static void mac_mouse_dragged(id self, SEL _cmd, id event) {
    (void)_cmd;
    GranularSynth *synth = (GranularSynth *)objc_getAssociatedObject(self, "synth");
    if (!synth) return;

    CGPoint win_p = ((CGPoint (*)(id, SEL))objc_msgSend)(event, sel_registerName("locationInWindow"));
    CGPoint p = ((CGPoint (*)(id, SEL, CGPoint, id))objc_msgSend)(self, sel_registerName("convertPoint:fromView:"), win_p, (id)NULL);
    gui_handle_mouse_drag(synth, (float)p.x, (float)(GUI_HEIGHT - p.y));
}

static void mac_mouse_up(id self, SEL _cmd, id event) {
    (void)_cmd; (void)event;
    GranularSynth *synth = (GranularSynth *)objc_getAssociatedObject(self, "synth");
    if (synth) gui_handle_mouse_up(synth);
}
#endif

static bool gui_is_api_supported(const clap_plugin_t *p, const char *api, bool fl) {
    (void)p; if (fl) return false;
#if defined(__APPLE__)
    return strcmp(api, CLAP_WINDOW_API_COCOA) == 0;
#elif defined(_WIN32)
    return strcmp(api, CLAP_WINDOW_API_WIN32) == 0;
#else
    return false;
#endif
}

static bool gui_get_preferred_api(const clap_plugin_t *p, const char **api, bool *fl) {
    (void)p; *fl = false;
#if defined(__APPLE__)
    *api = CLAP_WINDOW_API_COCOA; return true;
#elif defined(_WIN32)
    *api = CLAP_WINDOW_API_WIN32; return true;
#else
    return false;
#endif
}

static bool gui_create(const clap_plugin_t *plugin, const char *api, bool fl) {
    (void)api; (void)fl;
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

static bool gui_set_scale(const clap_plugin_t *p, double s) { (void)p; (void)s; return true; }
static bool gui_get_size(const clap_plugin_t *p, uint32_t *w, uint32_t *h) { (void)p; *w = GUI_WIDTH; *h = GUI_HEIGHT; return true; }
static bool gui_can_resize(const clap_plugin_t *p) { (void)p; return false; }
static bool gui_get_resize_hints(const clap_plugin_t *p, clap_gui_resize_hints_t *h) { (void)p; (void)h; return false; }
static bool gui_adjust_size(const clap_plugin_t *p, uint32_t *w, uint32_t *h) { (void)p; *w = GUI_WIDTH; *h = GUI_HEIGHT; return true; }
static bool gui_set_size(const clap_plugin_t *p, uint32_t w, uint32_t h) { (void)p; return (w == GUI_WIDTH && h == GUI_HEIGHT); }

static bool gui_set_parent(const clap_plugin_t *plugin, const clap_window_t *window) {
    if (!plugin || !plugin->plugin_data || !window) return false;
    GranularSynth *synth = (GranularSynth *)plugin->plugin_data;

#if defined(__APPLE__)
    id parent_view = (id)window->cocoa;
    if (!parent_view) return false;

    static Class custom_view_class = Nil;
    if (!custom_view_class) {
        custom_view_class = objc_allocateClassPair(objc_getClass("NSView"), "LukyanGuiCustomView", 0);
        class_addMethod(custom_view_class, sel_registerName("mouseDown:"), (IMP)mac_mouse_down, "v@:@");
        class_addMethod(custom_view_class, sel_registerName("mouseDragged:"), (IMP)mac_mouse_dragged, "v@:@");
        class_addMethod(custom_view_class, sel_registerName("mouseUp:"), (IMP)mac_mouse_up, "v@:@");
        objc_registerClassPair(custom_view_class);
    }

    id my_view = ((id (*)(id, SEL))objc_msgSend)((id)custom_view_class, sel_registerName("alloc"));
    CGRect frame = CGRectMake(0, 0, GUI_WIDTH, GUI_HEIGHT);
    my_view = ((id (*)(id, SEL, CGRect))objc_msgSend)(my_view, sel_registerName("initWithFrame:"), frame);

    objc_setAssociatedObject(my_view, "synth", (id)synth, OBJC_ASSOCIATION_ASSIGN);

    ((void (*)(id, SEL, BOOL))objc_msgSend)(my_view, sel_registerName("setWantsLayer:"), (BOOL)1);
    ((void (*)(id, SEL, id))objc_msgSend)(parent_view, sel_registerName("addSubview:"), my_view);

    synth->gui.native_view = (void *)my_view;
    synth->gui.parent_window = (void *)parent_view;
    synth->gui.is_open = true;

    gui_render_frame(synth);
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

static bool gui_set_transient(const clap_plugin_t *p, const clap_window_t *w) { (void)p; (void)w; return true; }
static void gui_suggest_title(const clap_plugin_t *p, const char *t) { (void)p; (void)t; }

static bool gui_show(const clap_plugin_t *plugin) {
    if (!plugin || !plugin->plugin_data) return false;
    GranularSynth *synth = (GranularSynth *)plugin->plugin_data;
    synth->gui.is_open = true;
    gui_render_frame(synth);
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