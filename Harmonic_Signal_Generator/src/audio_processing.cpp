/*****************************************************************************
 * audio_processing.cpp
 *****************************************************************************/

/*
 32-bit fixed point Linkwitz-Riley crossover

 Author: Uwe Simmer
 Date: 19.08.2017

 Author: Mario Ghecea
 Date: 13.06.2018
 (Added UART & Sine Wave Generator with DMA)
  Some Principles Borrowed from :
  The book "Embedded Signal Processing with the Micro Signal Architecture"
		  By Woon-Seng Gan and Sen M. Kuo
		  Publisher: John Wiley and Sons, Inc.

 This software is public domain
*/

#include <math.h>
#include <stdio.h>
//#include <stdint>

#include "audio_processing.h"

// left input data from AD1871
short sCh0LeftIn[INPUT_SIZE];
// right input data from AD1871
short sCh0RightIn[INPUT_SIZE];
// left ouput data for AD1854
short sCh0LeftOut[INPUT_SIZE];
// right ouput data for AD1854
short sCh0RightOut[INPUT_SIZE];

// SPORT0 DMA transmit buffer
int iTxBuffer1[2*INPUT_SIZE*TOTAL_FRAME];
// SPORT0 DMA receive buffer
int iRxBuffer1[2*INPUT_SIZE*TOTAL_FRAME];

unsigned char ucLED;
unsigned char ucMode;

// create table (48 values of 1kHz sine with 48kHz sampling rate)
fract16 lookup_table[DATASIZE] = {
	#include "sine1k_small.dat"
};

// this table is necessary to avoid division operation
fract16 scaling[SCALESIZE] = {
	#include "scaler.dat"
};

int idx1,idx2,idx3,idx4;
int idxquad1,idxquad2,idxquad3,idxquad4;
volatile unsigned char freq, freqsweep;

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

	ucMode = SINE;
	idx1 = 0;
	idxquad1 = DATASIZE/4;  //90 degree phase diff
	freq = 1;

//    for (int i=0; i<num_sos; i++)
//        for (int j=0; j<num_states; j++)
//            hp_states[i][j] = lp_states[i][j] = 0;
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


void freq_up(void)
{
	if (ucMode == SINE)
	{
		if (freq < 10) freq++;
		else freq = 1;
	}
	else if (ucMode == SAWTOOTH)
	{
		if (freq < 5) freq++;
		else freq = 1;
	}
	else if (ucMode == SQUARE)
	{
		if (freq < 3) freq++;
		else freq = 1;
	}
}


void freq_down(void)
{
	if (ucMode == SINE)
	{
		if (freq > 1) freq--;
		else freq = 10;
	}
	else if (ucMode == SAWTOOTH)
	{
		if (freq > 1) freq--;
		else freq = 5;
	}
	else if (ucMode == SQUARE)
	{
		if (freq > 1) freq--;
		else freq = 3;
	}
}

void Process_Data(uint8_t mode)
{
	int i,j;
	static int counter = 0;

	ucMode = mode;

	if (ucMode == SINE)
	{
		for (i=0; i<INPUT_SIZE; i++)
		{
			sCh0LeftOut[i] = lookup_table[idx1];
			sCh0RightOut[i] = lookup_table[idxquad1]; //90 degree phase diff

			// index increment depends on the current freq
			idx1 = circindex(idx1, freq, DATASIZE);
			idxquad1 = circindex(idxquad1, freq, DATASIZE);
		}
	}
	else if (ucMode == SAWTOOTH)
	{
		for (i=0; i<INPUT_SIZE; i++)
		{
			// y_sawtooth = sin(x) + sin(2x)/2 + sin(3x)/3 + sin(4x)/4
			sCh0LeftOut[i] = -(lookup_table[idx1] +
						 multr_fr1x16(lookup_table[idx2],scaling[2]) +
						 multr_fr1x16(lookup_table[idx3],scaling[3]) +
						 multr_fr1x16(lookup_table[idx4],scaling[4]));

			sCh0RightOut[i] = -(lookup_table[idxquad1] +
						 multr_fr1x16(lookup_table[idxquad2],scaling[2]) +
						 multr_fr1x16(lookup_table[idxquad3],scaling[3]) +
						 multr_fr1x16(lookup_table[idxquad4],scaling[4]));

			// each index has different increment
			idx1 = circindex(idx1, freq, DATASIZE);
			idx2 = circindex(idx2, 2*freq, DATASIZE);
			idx3 = circindex(idx3, 3*freq, DATASIZE);
			idx4 = circindex(idx4, 4*freq, DATASIZE);

			idxquad1 = circindex(idxquad1, freq, DATASIZE);
			idxquad2 = circindex(idxquad2, 2*freq, DATASIZE);
			idxquad3 = circindex(idxquad3, 3*freq, DATASIZE);
			idxquad4 = circindex(idxquad4, 4*freq, DATASIZE);
		}
	}
	else if (ucMode == SQUARE)
	{
		for (i=0; i<INPUT_SIZE; i++)
		{
			// y_square = sin(x) + sin(3x)/3 + sin(5x)/5 + sin(7x)/7
			sCh0LeftOut[i] = -(lookup_table[idx1] +
						 multr_fr1x16(lookup_table[idx2],scaling[3]) +
						 multr_fr1x16(lookup_table[idx3],scaling[5]) +
						 multr_fr1x16(lookup_table[idx4],scaling[7]));

			sCh0RightOut[i] = -(lookup_table[idxquad1] +
						 multr_fr1x16(lookup_table[idxquad2],scaling[3]) +
						 multr_fr1x16(lookup_table[idxquad3],scaling[5]) +
						 multr_fr1x16(lookup_table[idxquad4],scaling[7]));

			idx1 = circindex(idx1, freq, DATASIZE);
			idx2 = circindex(idx2, 3*freq, DATASIZE);
			idx3 = circindex(idx3, 5*freq, DATASIZE);
			idx4 = circindex(idx4, 7*freq, DATASIZE);

			idxquad1 = circindex(idxquad1, freq, DATASIZE);
			idxquad2 = circindex(idxquad2, 3*freq, DATASIZE);
			idxquad3 = circindex(idxquad3, 5*freq, DATASIZE);
			idxquad4 = circindex(idxquad4, 7*freq, DATASIZE);
		}
	}
	else if (ucMode == SWEEP)
	{
		for (i=0; i<INPUT_SIZE; i++)
		{
			sCh0LeftOut[i] = lookup_table[idx1];
			sCh0RightOut[i] = lookup_table[idxquad1];

			idx1 = circindex(idx1, freqsweep, DATASIZE);
			idxquad1 = circindex(idxquad1, freqsweep, DATASIZE);
		}

		// each frequency lasts for 1 sec, tracked by this counter
		counter = circindex(counter, INPUT_SIZE, 100*INPUT_SIZE);
		if (counter == 0)
		{
			if (freqsweep < 10)
				freqsweep++;
			else
				freqsweep = 1;
		}
	}
}
