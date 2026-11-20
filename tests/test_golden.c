#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "granular.h"
#include "parameters.h"

#define TEST_SAMPLE_RATE 44100.0
#define TEST_FRAMES 44100
#define BLOCK_SIZE 128
#define EPSILON 0.005f

int main(int argc, char **argv) {
    bool generate_mode = false;
    const char *golden_file = "tests/golden_reference.bin";

    if (argc > 1 && strcmp(argv[1], "--generate") == 0) {
        generate_mode = true;
    }

    GranularSynth *synth = (GranularSynth *)calloc(1, sizeof(GranularSynth));
    if (!synth) {
        fprintf(stderr, "Failed to allocate synth instance\n");
        return 1;
    }

    granular_init(&synth->engine, TEST_SAMPLE_RATE);
    params_init_defaults(synth);

    granular_set_osc_gain(&synth->engine, 0, 0.8f);
    granular_set_osc_morph(&synth->engine, 0, 0.5f);
    granular_set_attack(&synth->engine, 50.0f);
    granular_set_release(&synth->engine, 300.0f);
    granular_set_grain_size(&synth->engine, 60.0f);
    granular_set_density(&synth->engine, 40.0f);

    granular_note_on(&synth->engine, 69, 440.0f);

    float *out_l = (float *)malloc(TEST_FRAMES * sizeof(float));
    float *out_r = (float *)malloc(TEST_FRAMES * sizeof(float));

    for (uint32_t i = 0; i < TEST_FRAMES; i += BLOCK_SIZE) {
        uint32_t frames_to_render = (TEST_FRAMES - i < BLOCK_SIZE) ? (TEST_FRAMES - i) : BLOCK_SIZE;
        
        if (i >= TEST_FRAMES / 2) {
            granular_note_off(&synth->engine, 69);
        }

        granular_render_block(&synth->engine, &out_l[i], &out_r[i], frames_to_render);
    }

    if (generate_mode) {
        golden_file = "tests/golden_reference.bin";
        FILE *f = fopen(golden_file, "wb");
        if (!f) {
            golden_file = "../tests/golden_reference.bin";
            f = fopen(golden_file, "wb");
        }
        if (!f) {
            fprintf(stderr, "Cannot write golden file: tests/golden_reference.bin\n");
            return 1;
        }
        fwrite(out_l, sizeof(float), TEST_FRAMES, f);
        fwrite(out_r, sizeof(float), TEST_FRAMES, f);
        fclose(f);
        printf("[GOLDEN GEN] Successfully generated reference\n");
    } else {
        FILE *f = fopen("tests/golden_reference.bin", "rb");
        if (!f) {
            f = fopen("../tests/golden_reference.bin", "rb");
        }
        if (!f) {
            fprintf(stderr, "[ERROR] Golden reference file missing: tests/golden_reference.bin\n");
            free(out_l); free(out_r); free(synth);
            return 1;
        }

        float *ref_l = (float *)malloc(TEST_FRAMES * sizeof(float));
        float *ref_r = (float *)malloc(TEST_FRAMES * sizeof(float));

        fread(ref_l, sizeof(float), TEST_FRAMES, f);
        fread(ref_r, sizeof(float), TEST_FRAMES, f);
        fclose(f);

        float max_diff = 0.0f;
        for (uint32_t i = 0; i < TEST_FRAMES; ++i) {
            float diff_l = fabsf(out_l[i] - ref_l[i]);
            float diff_r = fabsf(out_r[i] - ref_r[i]);
            if (diff_l > max_diff) max_diff = diff_l;
            if (diff_r > max_diff) max_diff = diff_r;

            if (diff_l > EPSILON || diff_r > EPSILON) {
                fprintf(stderr, "[FAILED] Mismatch at sample %u: diff L=%f, R=%f (threshold=%f)\n", 
                        i, diff_l, diff_r, EPSILON);
                free(ref_l); free(ref_r); free(out_l); free(out_r); free(synth);
                return 1;
            }
        }

        printf("[PASSED] Golden test matched! Max difference: %.8f (within tolerance %.6f)\n", max_diff, EPSILON);
        free(ref_l); free(ref_r);
    }

    free(out_l);
    free(out_r);
    free(synth);
    return 0;
}