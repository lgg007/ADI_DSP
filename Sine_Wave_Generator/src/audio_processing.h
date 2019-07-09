#ifndef _AUDIO_PROCESSING
#define _AUDIO_PROCESSING

#include <stdint.h>

#define INTERNAL_ADC_L0			0
#define INTERNAL_ADC_R0			1
#define INTERNAL_DAC_L0			0
#define INTERNAL_DAC_R0			1
//--------------------------------------------------------------------------//
// Global variables															//
//--------------------------------------------------------------------------//
#define INPUT_SIZE				480
#define TOTAL_FRAME				2
#define DATASIZE				48
#define SCALESIZE				8

extern short sCh0LeftIn[];
extern short sCh0RightIn[];
extern short sCh0LeftOut[];;
extern short sCh0RightOut[];

extern int iRxBuffer1[];
extern int iTxBuffer1[];

//led state
//extern unsigned char ucLED;

//mode
extern unsigned char ucMode;
#define	SINE		0x02
#define SAWTOOTH	0x01
#define SQUARE		0x03
#define SWEEP		0x00

extern fract16 lookup_table[];
extern fract16 scaling[];
extern int idx1,idx2,idx3,idx4;
extern int idxquad1,idxquad2,idxquad3,idxquad4;
extern volatile unsigned char freq, freqsweep;

void init_audio_processing(void);

void audio_processing(int32_t audio_buffer[], int num_chans);
void Process_Data(void);


#endif // _AUDIO_PROCESSING
