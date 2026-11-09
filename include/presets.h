#ifndef PRESETS_H
#define PRESETS_H

#include "parameters.h"
#include <stdbool.h>

#define NUM_FACTORY_PRESETS 4
#define NUM_USER_PRESETS 5

typedef struct {
    char name[32];
    double values[PARAM_COUNT];
} Preset;

extern Preset g_factory_presets[NUM_FACTORY_PRESETS];

void presets_init_factory(GranularSynth *synth);

void presets_load_state(GranularSynth *synth, const Preset *preset);

// Сохранение и загрузка через файловый диалог
bool presets_save_to_file(const GranularSynth *synth, const char *filepath);
bool presets_load_from_file(GranularSynth *synth, const char *filepath);

#endif