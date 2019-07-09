/*****************************************************************************
 * ASM2.c
 *****************************************************************************/

/* Program ASM 2 flashes LED0, which is connected to pin 3 of the GPIO port C (PC3). 
It takes #define statements from the header file "defBF706.h". Note it only uses 
two, i.e. the port direction register (REG_PORTC_DIR_SET) and the port data register 
(REG_PORTC_DATA). First, it enables output on pin3 of port C. 
Next it enters a loop to pulse the pin. Note: the pin must be low for the LED  to turn 
(since the LED's cathode is connected to the pin). There is an error in the EVM manual
 which states the pin must be high.  
 
Author: Patrick Gaydecki Date  : 01.10.2016  */ 

 
.section program; 
.align 4; 
.global _ledon;
.global _ledoff; 
.extern _adi_initComponents;
 
#define REG_PORTC_DIR_SET               0x2004011C         /* PORTC Port x GPIO Direction Set Register */
#define REG_PORTC_DATA                  0x2004010C         /* PORTC Port x GPIO Data Register */  

_ledon: 
	//p3=r0; // store the passed in toggle state param in r0     
	r0=0;     
	r1=b#1000; 	 
// Set the direction and data registers.     
	p0=REG_PORTC_DIR_SET;     
	p1=REG_PORTC_DATA; 
// Set bit 3, port c as output.      
	[p0]=r1; 
	// Turn on LED0.     
	[p1]=r0;
	rts; 
._ledon.end:    	 
_ledoff:
	p0=REG_PORTC_DIR_SET;     
	p1=REG_PORTC_DATA;  
	r0=0;     
	r1=b#1000;
	[p0]=r1;
// Turn off LED0.     
	[p1]=r1;	     
	rts; 
._ledoff.end:
