/*****************************************************************************
* audio.cpp
*****************************************************************************/

/*
Program demonstrates a simple audio in-out capability of the ADSP-BF706 EZ-KIT Mini
evaluation system using a sample-by-sample algorithm. The program is completely
self-contained, using only the header cdefBF706.h for the addresses of the BF706
registers.

Author: Patrick Gaydecki
Date: 15.12.2016

Extension: audio-processing is called by the interrupt handler of the SPORT0
receive DMA interrupt. No polling is needed. The main loop can be used to 
process other tasks.

Author: Uwe Simmer
Date: 19.08.2017

This software is public domain
*/

#include <stdio.h>
#include <cdefBF706.h>
#include <services/int/adi_int.h>
#include "adi_initialize.h"
#include "audio_processing.h"
#include "audio.h"

void SPORT0_RX_interrupt_handler(uint32_t iid, void *handlerArg);

#define BUFFER_SIZE 2                       // Size of buffer to be transmitted
static int32_t SP0A_buffer[BUFFER_SIZE];    // DMA TX buffer
static int32_t SP0B_buffer[BUFFER_SIZE];    // DMA RX buffer
static int32_t audio_buffer[BUFFER_SIZE];   // Audio processing buffer

static bool gbFilterOn = false;

// Function write_TWI is a simple driver for the TWI. Refer to page 26–14 onwards
// of the ADSP-BF70x Blackfin+ Processor Hardware Reference, Revision 1.0.
void write_TWI(uint16_t reg_add, uint8_t reg_data)
{
    int n;
    reg_add = (reg_add<<8) | (reg_add>>8);  // Reverse low order and high order bytes
    *pREG_TWI0_CLKDIV = 0x3232;             // Set duty cycle
    *pREG_TWI0_CTL = 0x8c;                  // Set prescale and enable TWI
    *pREG_TWI0_MSTRADDR = 0x38;             // Address of CODEC
    *pREG_TWI0_TXDATA16 = reg_add;          // Address of register to set, LSB then MSB
    *pREG_TWI0_MSTRCTL = 0xc1;              // Command to send three bytes and enable transmit
    for (n = 0; n < 8000; n++) {}           // Delay since CODEC must respond
    *pREG_TWI0_TXDATA8 = reg_data;          // Data to write
    for (n = 0; n < 10000; n++) {}          // Delay
    *pREG_TWI0_ISTAT = 0x050;               // Clear TXERV interrupt
    for (n = 0; n < 10000; n++) {}          // Delay
    *pREG_TWI0_ISTAT = 0x010;               // Clear MCOMP interrupt
}

// Function configure_CODEC initializes the ADAU1761 CODEC. Refer to the control register
// descriptions, page 51 onwards of the ADAU1761 data sheet.
void configure_CODEC()
{
    write_TWI(0x4000, 0x01);                // Enable master clock, disable PLL
    write_TWI(0x400a, 0x0b);                // Set left line-in gain to 0 dB
    write_TWI(0x400c, 0x0b);                // Set right line-in gain to 0 dB
    write_TWI(0x4015, 0x01);                // Set serial port master mode
    write_TWI(0x4017, 0x00);                // Set CODEC default sample rate, 48 kHz
    write_TWI(0x4019, 0x63);                // Set ADC to on, both channels
    write_TWI(0x401c, 0x21);                // Enable left channel mixer
    write_TWI(0x401e, 0x41);                // Enable right channel mixer
    write_TWI(0x4023, 0xe7);                // Set left headphone volume to 0 dB
    write_TWI(0x4024, 0xe7);                // Set right headphone volume to 0 dB
    write_TWI(0x4029, 0x03);                // Turn on power, both channels
    write_TWI(0x402a, 0x03);                // Set both DACs on
    write_TWI(0x40f2, 0x01);                // DAC gets L, R input from serial port
    write_TWI(0x40f3, 0x01);                // ADC sends L, R input to serial port
    write_TWI(0x40f9, 0x7f);                // Enable all clocks
    write_TWI(0x40fa, 0x03);                // Enable all clocks
}

// Function init_interrupts enables the System Event Controller and installs the
// SPORT0 receive DMA interrupt handler
void init_interrupts(void)
{
    *pREG_SEC0_GCTL  = ENUM_SEC_GCTL_EN;    // Enable the System Event Controller (SEC)
    *pREG_SEC0_CCTL0 = ENUM_SEC_CCTL_EN;    // Enable SEC Core Interface (SCI)

    // Set SPORT0_B DMA interrupt handler
    adi_int_InstallHandler(INTR_SPORT0_B_DMA, SPORT0_RX_interrupt_handler, 0, true);
}

void restart_interrupts(void)
{
	 // Set SPORT0_B DMA interrupt handler
	 adi_int_InstallHandler(INTR_SPORT0_B_DMA, SPORT0_RX_interrupt_handler, 0, true);

}

// Function init_SPORT_DMA initializes the SPORT0 DMA0 and DMA1 in autobuffer mode.
// Refer to pages 19–39, and 19–49 of the ADSP-BF70x Blackfin+ Processor Hardware
// Reference, Revision 1.0.
void init_SPORT_DMA(void)
{
    // SPORT0_A DMA Initialization
    *pREG_DMA0_CFG       = 0x00001220;      // SPORT0 TX, FLOW = autobuffer, MSIZE = PSIZE = 4 bytes
    *pREG_DMA0_ADDRSTART = SP0A_buffer;     // points to start of SPORT0_A buffer
    *pREG_DMA0_XCNT      = BUFFER_SIZE;     // no. of words to transmit
    *pREG_DMA0_XMOD      = 4;               // stride

    // SPORT0_B DMA Initialization
    *pREG_DMA1_CFG       = 0x00101222;      // SPORT0 RX, DMA interrupt when x count expires
    *pREG_DMA1_ADDRSTART = SP0B_buffer;     // points to start of SPORT0_B buffer
    *pREG_DMA1_XCNT      = BUFFER_SIZE;     // no. of words to receive
    *pREG_DMA1_XMOD      = 4;               // stride
}

// Function configure_sport initializes the SPORT0. Refer to pages 31-55, 31-63,
// 31-72 and 31-73 of the ADSP-BF70x Blackfin+ Processor Hardware Reference, Revision 1.0.
void configure_SPORT()
{
    *pREG_SPORT0_CTL_A = 0x02011972;        // Set up SPORT0 (A) as TX to CODEC, I2S, 24 bits
    *pREG_SPORT0_DIV_A = 0x00400001;        // 64 bits per frame, clock divisor of 1
    *pREG_SPORT0_CTL_B = 0x00011972;        // Set up SPORT0 (B) as RX from CODEC, I2S, 24 bits
    *pREG_SPORT0_DIV_B = 0x00400001;        // 64 bits per frame, clock divisor of 1
}

// Function enable_SPORT enables DMA first and then SPORT
void enable_SPORT(void)
{
    *pREG_DMA0_CFG |= ENUM_DMA_CFG_EN;
    *pREG_DMA1_CFG |= ENUM_DMA_CFG_EN;
    __builtin_ssync();

    *pREG_SPORT0_CTL_B |= 1;
    *pREG_SPORT0_CTL_A |= 1;
    __builtin_ssync();
}

// Function disable_SPORT disables SPORT first and then DMA
void disable_SPORT(void)
{
    *pREG_SPORT0_CTL_B &= ~1;
    *pREG_SPORT0_CTL_A &= ~1;
    __builtin_ssync();

    *pREG_DMA0_CFG &= ~ENUM_DMA_CFG_EN;
    *pREG_DMA1_CFG &= ~ENUM_DMA_CFG_EN;
    __builtin_ssync();
}

// Function SPORT0_RX_interrupt_handler is called after a complete
// frame of input data has been received.
void SPORT0_RX_interrupt_handler(uint32_t iid, void *handlerArg)
{

    audio_buffer[0] = SP0B_buffer[0] << 8;
    audio_buffer[1] = SP0B_buffer[1] << 8;

    if (gbFilterOn == true)
    	audio_processing(audio_buffer, 2);

    SP0A_buffer[0] = audio_buffer[0] >> 8;
    SP0A_buffer[1] = audio_buffer[1] >> 8;

    adi_int_UninstallHandler(iid);
}

//-------------------------------------------------------------------------------------------------------

void initialize_drivers(bool bfilterOn)
{
	gbFilterOn = bfilterOn;
	adi_initComponents();
	init_audio_processing();
	init_interrupts();
	configure_CODEC();
	init_SPORT_DMA();
	configure_SPORT();
	enable_SPORT();
}
