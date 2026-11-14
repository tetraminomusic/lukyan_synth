#include "presets.h"
#include <stdio.h>
#include <string.h>

void presets_init_factory(GranularSynth *synth) {
    (void)synth;
}

void presets_load_state(GranularSynth *synth, const Preset *preset) {
    if (!synth || !preset) return;
    for (int i = 0; i < PARAM_COUNT; ++i) {
        params_apply_value(synth, i, preset->values[i]);
    }
}

bool presets_save_to_file(const GranularSynth *synth, const char *filepath) {
    if (!synth || !filepath) return false;
    
    FILE *f = fopen(filepath, "w");
    if (!f) return false;
    
    fprintf(f, "# LUKYAN SYNTH PRESET FILE\n");
    fprintf(f, "# Format: ParamID Value\n");
    
    for (int i = 0; i < PARAM_COUNT; ++i) {
        fprintf(f, "%d %f\n", i, synth->values[i]);
    }
    
    fclose(f);
    return true;
}

bool presets_load_from_file(GranularSynth *synth, const char *filepath) {
    if (!synth || !filepath) return false;
    
    FILE *f = fopen(filepath, "r");
    if (!f) return false;
    
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        
        int param_id = -1;
        double value = 0.0;
        
        if (sscanf(line, "%d %lf", &param_id, &value) == 2) {
            if (param_id >= 0 && param_id < PARAM_COUNT) {
                params_apply_value(synth, param_id, value);
            }
        }
    }
    
    fclose(f);
    return true;
}