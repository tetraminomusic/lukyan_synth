#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "parameters.h"
#include "presets.h"

int main(void) {
    GranularSynth *synth = (GranularSynth *)calloc(1, sizeof(GranularSynth));
    assert(synth != NULL);

    params_init_defaults(synth);

    synth->values[PARAM_GRAIN_SIZE] = 142.5;
    synth->values[PARAM_DENSITY] = 73.2;
    synth->values[PARAM_CUTOFF] = 4321.0;
    synth->values[PARAM_RESONANCE] = 3.14;
    synth->values[PARAM_REVERB_MIX] = 0.88;

    const char *test_path = "tests/test_temp_preset.lukyan";

    bool saved = presets_save_to_file(synth, test_path);
    assert(saved == true);

    params_init_defaults(synth);
    assert(synth->values[PARAM_GRAIN_SIZE] != 142.5);

    bool loaded = presets_load_from_file(synth, test_path);
    assert(loaded == true);

    assert(fabs(synth->values[PARAM_GRAIN_SIZE] - 142.5) < 1e-4);
    assert(fabs(synth->values[PARAM_DENSITY] - 73.2) < 1e-4);
    assert(fabs(synth->values[PARAM_CUTOFF] - 4321.0) < 1e-4);
    assert(fabs(synth->values[PARAM_RESONANCE] - 3.14) < 1e-4);
    assert(fabs(synth->values[PARAM_REVERB_MIX] - 0.88) < 1e-4);

    remove(test_path);
    free(synth);

    printf("[PASSED] Preset roundtrip test: File format serialization verified\n");
    return 0;
}