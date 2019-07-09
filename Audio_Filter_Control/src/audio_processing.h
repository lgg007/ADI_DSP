#ifndef _AUDIO_PROCESSING
#define _AUDIO_PROCESSING

#include <stdint.h>

void init_audio_processing(void);

void audio_processing(int32_t audio_buffer[], int num_chans);

#endif // _AUDIO_PROCESSING
