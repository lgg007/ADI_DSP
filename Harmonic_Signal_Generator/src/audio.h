/*****************************************************************************
 * audio.h
 *****************************************************************************/

#ifndef __AUDIO_H__
#define __AUDIO_H__

/* Add your custom header content here */
void write_TWI(uint16_t, uint8_t);
void configure_CODEC(void);
void init_interrupts(void);
void configure_SPORT(void);
void init_SPORT_DMA(void);
void init_SPORT_DMA2(void);
void enable_SPORT(void);
void disable_SPORT(void);
void initialize_drivers(bool bfilterOn);
void restart_interrupts(void);
void frequp(void);
void freqdn(void);

extern void freq_up(void);
extern void freq_down(void);

void modeup(void);
void modedn(void);

#endif /*__AUDIO_H__ */
