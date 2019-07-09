/*********************************************************************************

Copyright(c) 2012 Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.

*********************************************************************************/
/*!
* @file      char_echo.c
*
* @brief     This is the primary source file for UART character echo example.
* 			 Ported from BF707 & BF533 platforms...
* 			 Modify as you wish but do not violate copyrights while distributing...
* 			 I take no responsibility for this code. See Analog Devices License Agreements
* Ported by : Mario Ghecea
* Date		: 5/25/2018
* E-Mail    : dreamsmatrix@gmail.com
*
*
*
*/

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rules_all:"suppress all MISRA rules for test")
#endif /* _MISRA_RULES */


#include "char_echo.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <services\int\adi_int.h>
#include <drivers\uart\adi_uart.h>
#include <services\pwr\adi_pwr.h>

/* ADI initialization header */
#include "adi_initialize.h"

#define USE_UART_NR     0

/* Number of Tx and Rx buffers */
#define NUM_BUFFERS     2
// level of verbosity
#define __VERBOSITY__   2
#define STRINGSIZE		512
char strbuf[STRINGSIZE];

#define _PROMPT_        "bf\\>"
#define _RESETSCREEN_   "\f\e[0m"
#define _CLEARSCREEN_   "\e[2J"
#define _ERASELINE_     "\e[K"
//    #define _NEWLINE_       "\n\r" /* carriage return is inserted automatic */
#define _NEWLINE_       "\n"
#define _CARRIAGE_      "\r"
#define _VTAB_          "\v"
#define _HTAB_          "\t"
#define _CURSORUP_      "\e[A"
#define _CURSORDN_      "\e[B"
#define _CURSORFW_      "\e[C"
#define _CURSORBW_      "\e[D"
#define _CURSORUPX_     "\e[%dA" /* requires the number of lines as the 1st parameter */
#define _CURSORDNX_     "\e[%dB" /* requires the number of lines as the 1st parameter */
#define _CURSORFWX_     "\e[%dC" /* requires the number of lines as the 1st parameter */
#define _CURSORBWX_     "\e[%dD" /* requires the number of lines as the 1st parameter */
#define _INVERSEON_     "\e[7m"
#define _INVERSEOFF_    "\e[27m"
#define _NORMALTEXT_    "\e[0m"
#define _BOLDTEXT_      "\e[1m"
#define _ITALICTEXT_    "\e[3m"
#define _BLINKTEXT_     "\e[5m"
#define _REDTEXT_       "\e[31m"
#define _GREENTEXT_     "\e[32m"
#define _YELLOWTEXT_    "\e[33m"
#define _BLUETEXT_      "\e[34m"
#define _MAGENTATEXT_   "\e[35m"
#define _CYANTEXT_      "\e[36m"
#define _WHITETEXT_     "\e[37m"
#define _BLACKTEXT_     "\e[30m"
#define _TEST_          "\e[=3h"

#define _NL_    _NEWLINE_
#define _CR_    _CARRIAGE_
#define _EL_    _ERASELINE_
#define _CS_    _CLEARSCREEN_

static char crsuw[] = { '\e', '[', 'A' }; // Cursor Up
static char crsdw[] = { '\e', '[', 'B' }; // Cursor Down
static char crsfw[] = { '\e', '[', 'C' }; // Cursor Forward
static char crsbw[] = { '\e', '[', 'D' }; // Cursor Backward

static char crstb[] = { '\e', '[', '4', 'C' }; // Tab: Cursor Forward by 4

short uprintf(unsigned char UartNum, const char *fmt, ...);

#if (__VERBOSITY__ > 0)
#define DEBUG(n, args...)\
    do {\
        if (__VERBOSITY__ >= n) { uprintf(USE_UART_NR,_NORMALTEXT_ args); }\
    } while (0)
#else
#define DEBUG(n, args...) do { } while(0)
#endif

/* UART Handle */
static ADI_UART_HANDLE  ghUART;

/* Rx and Tx buffers */
static uint8_t RxBuffer[NUM_BUFFERS];
static uint8_t TxBuffer;

/* Memory required for operating UART in interrupt mode */
static uint8_t  gUARTMemory[ADI_UART_BIDIR_DMA_MEMORY_SIZE];

/* Sets the Software controlled switches */
extern void ConfigSoftSwitches(void);

// This is the original Blackfin sample logo I imported from the BF533 source...
void WelcomeMessage(void)
{
    DEBUG(1,""_RESETSCREEN_"");
    DEBUG(1,"                                         ,8888oo."_NL_"");
    DEBUG(1,"                                          Y8888888o."_NL_"");
    DEBUG(1,"                                           Y888888888L"_NL_"");
    DEBUG(1,"                                            Y8888888888L"_NL_"");
    DEBUG(1,"                                             888888888888L"_NL_"");
    DEBUG(1,"                                             d8888888888888."_NL_"");
    DEBUG(1,"                                             ]888888888888888."_NL_"");
    DEBUG(1,"                                             ]888888888PP''''"_NL_"");
    DEBUG(1,"                                             ]8888888P'          ."_NL_"");
    DEBUG(1,"    ,ooooo.                                  ]88888P    ,ooooooo88b."_NL_"");
    DEBUG(1,"   ,8888888p                                 ]8888P   ,8888888888888o"_NL_"");
    DEBUG(1,"   d88P'888[ ooo'    oooo.  _oooo.  ooo  oop ]888P    `'' ,888P8888888o"_NL_"");
    DEBUG(1,"  ,888 J88P J88P    d8P88  d88P888 ,88P,88P  `P''       ,88P' ,888888888L"_NL_"");
    DEBUG(1,"  d888o88P' 888'   ,8P,88 ,88P 88P d88b88P    __    d88888[ _o8888888P8888."_NL_"");
    DEBUG(1," ,8888888. J88P    88'd8P d88  '' ,88888P    d8'   d888888888888888P   88PYb."_NL_"");
    DEBUG(1," d88P 888P 888'   d8P 88[,88P ,o_ d88888.   ,8P   d8888P'  88888P'    P'   ]8b_"_NL_"");
    DEBUG(1,",888']888'J88P   d888888 d88'J88',88Pd88b   JP   d888P     888P',op    ,   d88P"_NL_"");
    DEBUG(1,"d8888888P d88bo.,88P'Y8P 888o88P d88']88b   d'  ,8P'_odb   ''',d8P  _o8P  ,P',,d8L"_NL_"");
    DEBUG(1,"PPPPPPP' `PPPPP YPP  PPP `PPPP' <PPP `PPP  ,P   88o88888.  ,o888'  o888     d888888."_NL_"");
    DEBUG(1,"                                           d'  d888888888888888L_o88888L_,o888888888b."_NL_"");
    DEBUG(1,"                                          dP  d888888888888888888888888888888888888888o"_NL_"");
    DEBUG(1,"                                        dd8' ,888888888888888888888888888888888888888888L"_NL_"");
    DEBUG(1,"                                       d88P  d88888888888888888888888888888888888888888888."_NL_"");
    DEBUG(1,""_VTAB_"");
    DEBUG(1,"File: %s | Line: %d | Function: %s"_NL_"",__FILE__,__LINE__,__func__);
    DEBUG(1,"Build Date: %s | Build Time: %s"_NL_"",__DATE__,__TIME__);
    DEBUG(1,"Ported By: Mario Ghecea for BF706"_NL_"");
    DEBUG(1,"Date	  : 5/25/2018"_NL_"");
    DEBUG(1,"E-Mail   : dreamsmatrix@gmail.com"_NL_"");
    DEBUG(1,""_VTAB_"");
    DEBUG(1,"\n");
}

//***************************************
//*
//* Function Name : short UartPutc(unsigned char UartNum, char c)
//* Description   : This function transmits a character,
//*                 the THRE bit in the LSR register is polled to ensure that THR is empty.
//*
//* Parameters    : The character to transmit, UART number
//* Returns       : NULL. If the UartPutc function is unsuccessful, a negative value is returned.
//* Globals       : none
//*
short UartPutc(unsigned char UartNum, char c)
{
	ADI_UART_RESULT    eResult;
    if (UartNum!=0) { printf("%s(): UART%d is not available.\n",__func__,UartNum); return -1; }

    /* Write back the character */
	if((eResult = adi_uart_Write(ghUART,
			&c,
			1
	)) != ADI_UART_SUCCESS)
	{
		REPORT_ERROR("Could not do a write 0x%08X \n", eResult);
		return FAILED;
	}

    return 0;
}

//***************************************
//*
//* Function Name : short UartPuts(unsigned char UartNum, char *c)
//* Description   : This function transmits a byte string.
//*                 The string must be NULL-terminated (C-style).
//*                 A newline / carriage return character will terminate the transfer as well.
//*
//* Parameters    : UART number, pointer to the string to transmit
//* Returns       : NULL. If the UartPuts function is unsuccessful, a negative value is returned.
//* Globals       : none
//*
short UartPuts(unsigned char UartNum, char *c)
{
    while (*c) {
        UartPutc(UartNum, *c);
        if (*c == '\n') { // 0x0A = newline?
            UartPutc(UartNum, '\r'); // 0x0D = insert carriage return
            return 0;
            }
        else if (*c == '\r') { // 0x0D = carriage return?
            UartPutc(UartNum, '\n'); // 0x0A = insert newline
            return 0;
            }
        else { c++; }
        }

    return 0;
}

//***************************************
//*
//* Function Name : short uprintf(unsigned char UartNum, const char *format, /* args */ ...)
//* Description   : The uprintf function places output on the UART in a form
//*                 specified by format. The uprintf function is equivalent to
//*                 udprintf with DMA based UART transfer. The argument format
//*                 contains a set of conversion specifiers, directives, and
//*                 ordinary characters that are used to control how the data
//*                 is formatted. Refer to 'fprintf' in the C Run-Time Library
//*                 Reference for a description of the valid format specifiers.
//*
//* Parameters    : uchar UartNum, const char *format, arguements
//* Returns       : The uprintf function returns the number of
//*                 characters transmitted, excluding 'carriage return'.
//*                 If the uprintf function is unsuccessful, a negative value is returned.
//* Globals       : char strbuf[STRINGSIZE]
//*
//* Control Commands:
//*                     '\e' escape character. It maps to the ASCII Escape code, 27
//*                     '\f' form feed/flush screen
//*                     '\n' new line
//*                     '\r' carriage return
//*                     '\t' horizontal tab
//*                     '\v' vertical tab
//*
short uprintf(unsigned char UartNum, const char *fmt, ...)
{
    short outsize;
    va_list ap;

    /* return failure if format is NULL pointer */
    if (!fmt) { return -1; }
    /* return failure if string exceeds buffer size, two additional characters required: carriage return & NULL termination */
    if (sizeof(strbuf)<(strlen(fmt)-1)) { return -1; }

    va_start (ap,fmt);
    outsize = vsprintf(strbuf,fmt,ap);
    /* insert carriage return, if newline is detected */
    if (strbuf[outsize-1] == '\n') { strbuf[outsize] = '\r'; }
    /* insert newline, if only carriage return is detected */
    else if (strbuf[outsize-1] == '\r') { strbuf[outsize] = '\n'; }
    /* ensure NULL-termination (C-style) */
    strbuf[outsize+1] = NULL;
    va_end (ap);

    UartPuts(UartNum,strbuf);

    return outsize;
}


int main(void)
{
	/* UART return code */
	ADI_UART_RESULT    eResult;
	/* Tx size */
	uint32_t           nTxSize = 1;
	/* Flag which indicates whether to stop the program */
    _Bool bStopFlag = false;

    adi_initComponents(); /* auto-generated code */

    printf("UART Char echo blocking mode example \n");

    /* Initialize Power service */
#if defined (__ADSPBF707_FAMILY__) || defined (__ADSPSC589_FAMILY__)
    if(adi_pwr_Init(0, CLKIN) != ADI_PWR_SUCCESS)
    {
        REPORT_ERROR("Failed to initialize power service \n");
        return FAILED;
    }
#else
    if(adi_pwr_Init(CLKIN, CORE_MAX, SYSCLK_MAX, VCO_MIN) != ADI_PWR_SUCCESS)
    {
        REPORT_ERROR("Failed to initialize power service \n");
        return FAILED;
    }
#endif
    /*
     * Initialize UART
     */
    /* Open UART driver */
    if((eResult = adi_uart_Open(UART_DEVICE_NUM,
                                 ADI_UART_DIR_BIDIRECTION,
                                 gUARTMemory,
                                 ADI_UART_BIDIR_DMA_MEMORY_SIZE,
                                 &ghUART)) != ADI_UART_SUCCESS)
    {
    	REPORT_ERROR("Could not open UART Device 0x%08X \n", eResult);
    	return FAILED;
    }
    /* Set the UART Mode */
    if((eResult = adi_uart_SetMode(ghUART,
                                    ADI_UART_MODE_UART
                                   )) != ADI_UART_SUCCESS)
    {
    	REPORT_ERROR("Could not set the Mode 0x%08X \n", eResult);
    	return FAILED;
    }

    /* Set UART Baud Rate */
    if((eResult = adi_uart_SetBaudRate(ghUART,
    									BAUD_RATE
                                        )) != ADI_UART_SUCCESS)
    {
    	REPORT_ERROR("Could not set the Baud Rate 0x%08X \n", eResult);
    	return FAILED;
    }

    /* Set number of stop bits */
    if((eResult = adi_uart_SetNumStopBits(ghUART,
                                            ADI_UART_ONE_STOPBIT
                                         )) != ADI_UART_SUCCESS)
    {
    	REPORT_ERROR("Could not set the stop bits 0x%08X \n", eResult);
    	return FAILED;
    }

    /* Set number of stop bits */
    if((eResult = adi_uart_SetWordLen(ghUART,
                                          ADI_UART_WORDLEN_8BITS
                                         )) != ADI_UART_SUCCESS)
    {
    	REPORT_ERROR("Could not set word length 0x%08X \n", eResult);
    	return FAILED;
    }

    WelcomeMessage();

    printf("Setup Hyperterminal as described in Readme file. \n");
    printf("Press any key on the key board and notice the character  being echoed to Hyperterminal. \n");
    printf("\n Press return key to stop the program.\n");

    /* UART processing loop */
    while(bStopFlag == false)
    {
        /* Read a character */
        if((eResult = adi_uart_Read(ghUART,
                &RxBuffer[0],
                1
        )) != ADI_UART_SUCCESS)
        {
            REPORT_ERROR("Could not do a read 0x%08X \n", eResult);
            return FAILED;
        }

        /* If return character is pressed, write back \n character along with \r */
        if(RxBuffer[0] == '\r')
        {
            RxBuffer[1] = '\n';

            /* Transmit two characters in this case */
            nTxSize     = 2u;

            /* Stop the program upon receiving carriage return */
            bStopFlag = true;
        }

        /* Write back the character */
        if((eResult = adi_uart_Write(ghUART,
                &RxBuffer[0],
                nTxSize
        )) != ADI_UART_SUCCESS)
        {
            REPORT_ERROR("Could not do a write 0x%08X \n", eResult);
            return FAILED;
        }
     }

    /* Close the UART */
    if((eResult = adi_uart_Close(ghUART)) != ADI_UART_SUCCESS)
    {
        REPORT_ERROR("Could not close UART driver 0x%08X \n", eResult);
        return FAILED;
    }

    return PASSED;
}


#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */
