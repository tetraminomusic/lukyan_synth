#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include <stdint.h>

#define NUM_WAVEFORMS 4
#define SAMPLE_BUFFER_SIZE 96000
#define BASE_ROOT_FREQ 110.0f

void oscillator_init_buffers(double sample_rate);
float oscillator_read_morphed(float pos, float morph);

#endif
