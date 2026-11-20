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

    srand(42);
    granular_init(&synth->engine, TEST_SAMPLE_RATE);
    params_init_defaults(synth);

    granular_set_osc_gain(&synth->engine, 0, 0.8f);
    granular_set_osc_morph(&synth->engine, 0, 0.0f);
    granular_set_attack(&synth->engine, 10.0f);
    granular_set_release(&synth->engine, 100.0f);
    granular_set_grain_size(&synth->engine, 50.0f);
    granular_set_density(&synth->engine, 30.0f);
    granular_set_spray(&synth->engine, 0.0f);

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

        double mse_l = 0.0, mse_r = 0.0;
        for (uint32_t i = 0; i < TEST_FRAMES; ++i) {
            double err_l = (double)out_l[i] - (double)ref_l[i];
            double err_r = (double)out_r[i] - (double)ref_r[i];
            mse_l += err_l * err_l;
            mse_r += err_r * err_r;
        }
        double rmse_l = sqrt(mse_l / TEST_FRAMES);
        double rmse_r = sqrt(mse_r / TEST_FRAMES);

        printf("[GOLDEN] RMSE Left: %.8f, Right: %.8f\n", rmse_l, rmse_r);

        if (rmse_l > 0.1 || rmse_r > 0.1) {
            fprintf(stderr, "[FAILED] Root Mean Square Error too high: L=%f, R=%f\n", rmse_l, rmse_r);
            free(ref_l); free(ref_r); free(out_l); free(out_r); free(synth);
            return 1;
        }

        printf("[PASSED] Golden test matched via RMS error validation!\n");
        free(ref_l); free(ref_r);
    }

    free(out_l);
    free(out_r);
    free(synth);
    return 0;
}