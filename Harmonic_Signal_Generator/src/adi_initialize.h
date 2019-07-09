/*
** adi_initialize.h header file generated on August 1, 2017 at 10:37:50.
**
** Copyright (C) 2000-2017 Analog Devices Inc., All Rights Reserved.
**
** This file is generated automatically. You should not modify this source file,
** as your changes will be lost if this source file is re-generated.
*/

#ifndef __ADI_COMPONENT_INIT_H__
#define __ADI_COMPONENT_INIT_H__
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Declare "adi_initComponents()" */
int32_t adi_initComponents(void);
void initialize_drivers(bool bfilterOn);
int UART_Command_Processor(void);
void enable_SPORT(void);
void disable_SPORT(void);
void restart_interrupts(void);
void frequp(void);
void freqdn(void);
void modeup(void);
void modedn(void);
#ifdef __cplusplus
}
#endif

#endif /* __ADI_COMPONENT_INIT_H__ */

