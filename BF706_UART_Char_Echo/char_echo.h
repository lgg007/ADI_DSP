/*********************************************************************************

Copyright(c) 2012 Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.

*********************************************************************************/
/*!
* @file      char_echo.h
*
* @brief     This is the primary header file for UART character echo example.
*
*/

#ifndef CHAR_ECHO_H
#define CHAR_ECHO_H

/*
 * User configurations
 */

/* Un-comment this line to use DMA */
//#define USE_DMA_MODE

/* Baud rate to be used for char echo */
#define BAUD_RATE           115200u


/*
 * Macros used in the example
 */

/* Example result definitions */
#define FAILED              (-1)
#define PASSED              0

/* Macro for reporting errors */
#define REPORT_ERROR        printf



/*
 * Processor specific macros
 */

/* UART Device Number to test */
#define UART_DEVICE_NUM     0u

/* default power settings */
#define MHZTOHZ       (1000000)

#define DF_DEFAULT    (0x0)
#define MSEL_DEFAULT  (0x10)
#define SSEL_DEFAULT  (0x8)
#define CSEL_DEFAULT  (0x4)

#define CLKIN         (25 * MHZTOHZ)
#define CORE_MAX      (500 * MHZTOHZ)
#define SYSCLK_MAX    (250 * MHZTOHZ)
#define SCLK_MAX      (125 * MHZTOHZ)
#define VCO_MIN       (72 * MHZTOHZ)

#endif /* CHAR_ECHO_H */



