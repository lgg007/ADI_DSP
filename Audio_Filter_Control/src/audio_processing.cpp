/*****************************************************************************
 * audio_processing.cpp
 *****************************************************************************/

/*
 32-bit fixed point Linkwitz-Riley crossover

 Author: Uwe Simmer
 Date: 19.08.2017

 This software is public domain
*/

#include <math.h>
#include <stdio.h>
//#include <stdint>

#include "audio_processing.h"

// second order 400 Hz Butterworth highpass
static int32_t hp_coeffs[5] = {
    1034714276,
    -2069428553,
    1034714276,
    2068009541,
    -997105741
};

// second order 400 Hz Butterworth lowpass
static int32_t lp_coeffs[5] = {
    709506,
    1419012,
    709506,
    2068009541,
    -997105741
};

// 2 second order sections
const int num_sos = 2;
// 4 states per second order section
const int num_states = 4;
static int32_t hp_states[num_sos][num_states];
static int32_t lp_states[num_sos][num_states];

//------------------------------------------------------------------------------

inline int32_t iir32_sos(int32_t in, int32_t coeff[], int32_t mem[])
{
    // coeff[5] = {b0, b1, b2, -a1, -a2} 	/* Q 2.30 format */

    int64_t temp64;
    int32_t out;

    temp64 =  (int64_t) coeff[0] * in;      // b[0] * x[k]
    temp64 += (int64_t) coeff[1] * mem[0];  // b[1] * x[k-1]
    temp64 += (int64_t) coeff[2] * mem[1];  // b[2] * x[k-2]
    temp64 += (int64_t) coeff[3] * mem[2];  // a[1] * y[k-1]
    temp64 += (int64_t) coeff[4] * mem[3];  // a[2] * y[k-2]

    out = (int32_t) (temp64 >> 30);         // quantization

    mem[1] = mem[0];                        // 2nd non-recursive state
    mem[0] = in;                            // 1st non-recursive state
    mem[3] = mem[2];                        // 2nd recursive state
    mem[2] = out;                           // 1st recursive state

    return out;
}

//------------------------------------------------------------------------------

void init_audio_processing(void)
{
    for (int i=0; i<num_sos; i++)
        for (int j=0; j<num_states; j++)
            hp_states[i][j] = lp_states[i][j] = 0;
}

//------------------------------------------------------------------------------

void audio_processing(int32_t audio_buffer[], int num_chans)
{
    // 4th order Linkwitz-Riley highpass (left)
    audio_buffer[0] = iir32_sos(audio_buffer[0], hp_coeffs, hp_states[0]);
    audio_buffer[0] = iir32_sos(audio_buffer[0], hp_coeffs, hp_states[1]);

    // 4th order Linkwitz-Riley lowpass (right)
    audio_buffer[1] = iir32_sos(audio_buffer[1], lp_coeffs, lp_states[0]);
    audio_buffer[1] = iir32_sos(audio_buffer[1], lp_coeffs, lp_states[1]);
}

//------------------------------------------------------------------------------
