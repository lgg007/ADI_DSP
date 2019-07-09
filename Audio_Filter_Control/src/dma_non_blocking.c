
/*********************************************************************************/
/*!
* @file      char_dma_unblocked.c
*
* @brief     This is the non-blocking UART DMA terminal with audio cross-talk and FIR filter example
* 			 I have created as an add-on! (Kudos and comments are welcome, in general since it's volunteer work!)
* 			 I have previously ported blocking code from BF707 & BF533 platforms...
* 			 After Memorial day, I went one step further. Making non-blocking transfers via DMA!
* 			 Modify as you wish for your own study but do not violate copyrights while distributing...
* 			 I take no responsibility for damage resulting from use of this code.
* 			 Please - See Analog Devices License Agreements also...
*
* 			 This version collects the console incoming data at the PUTTY terminal in non-blocking
* 			 console command buffer and executes a blinking function to demonstrate a seamless
* 			 parallel execution, similar to what you would do inside any typical DSP algorithm...
* 			 Pay special attention on long delays between calls. The assembly call does not interrupt
* 			 inside the extensive delays! That is a feature that can be improved on...Remains as homework
* 			 assignment for the inquisitive developer to try out! I retain the rights to update this example
* 			 without warning!
*
* 			 If anything can be improved or you may have some serious questions, please notify me.
* 			 I'd like to see your implementations, so keep me posted! :)
* 			 I request that if you use and copy any of my code, that you do not remove
* 			 this informational header from this file!
*
* 			 Happy Dsp'ing!
*
* Author 	: Mario Ghecea
* Date		: 6/6/2018
* E-Mail    : dreamsmatrix@gmail.com
*
*
*
*/

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rules_all:"suppress all MISRA rules for test")
#endif /* _MISRA_RULES */


#include "dma_non_blocking.h"
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <services\int\adi_int.h>
#include <drivers\uart\adi_uart.h>
#include <services\pwr\adi_pwr.h>
//#include <cdefBF706.h>
#include <sys\cdefBF706.h>

/* ADI initialization header */
#include "adi_initialize.h"
#include "audio.h"
#include "audio_processing.h"

// Comment out #define USE_UART_DMA to only engage UART in interrupt mode!
#define USE_UART_DMA

#define MAX_LED_BLINK_RATE 	1000000
#define MIN_LED_BLINK_RATE 	1
#define DEFAULT_BLINK_RATE  100000
#define FILTER_BLINK_RATE	50000
#define USE_UART_NR     	0
#define MAX_SPORT_DELAY     10000

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

static uint8_t RxBuffer[2];
static uint8_t TxBuffer;
static uint8_t RxCommandBuffer[50];
static uint8_t idxRx = 0;
static uint32_t blinkCounter;


_Bool bRxBuffAvailable = true; // set to true to enter latch the first time...
 char * completedBuffer = NULL;

 enum eTERMINAL_ACTION TermAction;

/* Flag which indicates whether to stop the UART commands processor */
_Bool bStopFlag = false;


struct LedBlinker
{
	bool 		LedOn;		   	// Set LedOn = true to turn on; LedOn = false to turn off
	bool		FilterOn;		// Turn UweS crossover filter on or off
	bool 		AudioOn;		// Turns audio on or off by dissabling SPORT0 Interrupt...
	uint32_t 	LedBlinkRate; 	// Set LedBlinkRate to zero (0) to stop blinking
	bool		Exit;			// If this is turned on, go ahead and exit the driver
} *pBlinker;


enum eTERMINAL_ACTION {
	UNDECIDED = -1,
	LED_ON,
	LED_OFF,
	LED_BLINK_RATE,
	AUD_ON,
	AUD_OFF,
	FILT_ON,
	FILT_OFF,
	TERM,
	QUIT
};

static bool bError;
static bool bToggle = false;
static bool bOnce = true;
extern void ledon(void);
extern void ledoff(void);

/* Memory required for operating UART in interrupt mode */
static uint8_t  gUARTMemory[ADI_UART_BIDIR_DMA_MEMORY_SIZE];

int UART_Command_Processor(void);
static inline void FlashLEDIndicator (void);

static void CheckResult(ADI_UART_RESULT result) {
	if (result != ADI_UART_SUCCESS) {
		printf("UART failure\n");
		bError = true;
	}
}

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
    DEBUG(1,"Created By: Mario Ghecea for BF706"_NL_"");
    DEBUG(1,"Date	  : 5/28/2018"_NL_"");
    DEBUG(1,"E-Mail   : dreamsmatrix@gmail.com"_NL_"");
    DEBUG(1,""_VTAB_"");
    DEBUG(1,"\n");
    DEBUG(1, "Non-blocking DMA based terminal with audio crossover and FIR filters!" _NL_ "");
    DEBUG(1, "Commands are:" _NL_ "");
    DEBUG(1, "led on          (Turns LED on at default blink rate)" _NL_ "");
    DEBUG(1, "led off         (TurnsLED off at default blink rate)" _NL_ "");
    DEBUG(1, "aud on          (by Default is off, Turns audio on)" _NL_ "");
    DEBUG(1, "aud off         (Turns audio off)" _NL_"");
    DEBUG(1, "filt on         (Turns Crossover Filter on)" _NL_ "");
    DEBUG(1, "filt off        (Turns Crossover Filter off)" _NL_"");
    DEBUG(1, "rate=xxxxxxx    (Enter some LED blink rate without space from 1-1000000" _NL_ "");
    DEBUG(1, "term            (Resets the terminal banner)" _NL_ "");
    DEBUG(1, "quit            (Exits the terminal)" _NL_ "");
    DEBUG(1, "Lets get started!" _NL_ "");
    DEBUG(1, "Type your led terminal command followed by <ENTER>:" _NL_ "");
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

	TermAction = UNDECIDED;

    pBlinker = malloc(sizeof(struct LedBlinker));

    //LedBlinker * Blinker;


    pBlinker->FilterOn = false;
    pBlinker->LedBlinkRate = DEFAULT_BLINK_RATE;
    pBlinker->LedOn = false;


    // Turn the led back on if they turned filter on
    if (pBlinker->FilterOn == true || pBlinker->AudioOn == true)
    	pBlinker->LedOn = true;

    blinkCounter = pBlinker->LedBlinkRate;

    adi_initComponents(); /* auto-generated code */

    printf("UART Filter Control example \n");


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
#ifdef USE_UART_DMA
    if((eResult = adi_uart_Open(UART_DEVICE_NUM,
                                 ADI_UART_DIR_BIDIRECTION,
                                 gUARTMemory,
                                 ADI_UART_BIDIR_DMA_MEMORY_SIZE,
                                 &ghUART)) != ADI_UART_SUCCESS)
    {
    	REPORT_ERROR("Could not open UART Device 0x%08X \n", eResult);
    	return FAILED;
    }
#else // Run UART using interrupt mode only!
    if((eResult = adi_uart_Open(UART_DEVICE_NUM,
                                    ADI_UART_DIR_BIDIRECTION,
                                    gUARTMemory,
									ADI_UART_BIDIR_INT_MEMORY_SIZE,
                                    &ghUART)) != ADI_UART_SUCCESS)
       {
       	REPORT_ERROR("Could not open UART Device 0x%08X \n", eResult);
       	return FAILED;
       }
#endif
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

#ifdef USE_UART_DMA

    /* Enable the DMA associated with UART if UART is expected to work with DMA mode */
    if((eResult = adi_uart_EnableDMAMode(ghUART, true )) != ADI_UART_SUCCESS)
    {
    	REPORT_ERROR("Could not invoke Bi-Directional DMA Mode 0x%08X \n", eResult);
    	return FAILED;
    }
#endif

    WelcomeMessage();

    // Initialize our drivers
    //initialize_drivers();



    // Only pBlinker exit signal can break out of the super loop!

    while (pBlinker->Exit == false)
    {
    	//disable_SPORT();
    	UART_Command_Processor();

    	if (pBlinker->AudioOn == true || pBlinker->FilterOn == true)
    		restart_interrupts();
    }

    return 0;
}

int UART_Command_Processor(void)
{
	/* UART return code */
	ADI_UART_RESULT    eResult;
	/* Tx size */
	uint32_t           nTxSize = 1;

	while(bStopFlag == false)
	{
		// Enter this latch only when true
		if (bRxBuffAvailable == true)
		{
			bRxBuffAvailable = false;
			adi_uart_SubmitRxBuffer(ghUART, &RxBuffer[0], 1);
			//adi_uart_SubmitRxBuffer(ghUART, &RxBuffer[1], 1); // This should not be necessary
			/* enable the UART transfer */
			eResult = adi_uart_EnableRx(ghUART, true);
			CheckResult(eResult);
		}
		while(bRxBuffAvailable == false)
		{
//			eResult = adi_uart_EnableRx(ghUART, true);
//			CheckResult(eResult);

			eResult = adi_uart_IsRxBufferAvailable(ghUART, &bRxBuffAvailable);
			if(eResult != ADI_UART_SUCCESS)
			{
				REPORT_ERROR("Could not test RX Buffer availability 0x%08X \n", eResult);
				return FAILED;
			}

			// *******************************************************************
			// Do some meaningful work here like invoking your non-blocking algos...
			FlashLEDIndicator();
			// *******************************************************************

			if (bRxBuffAvailable == true)
				break;

			return false;
		}

	   /* Read a character */

	   if((eResult = adi_uart_GetRxBuffer(ghUART,
			   (void *)&completedBuffer
	   )) != ADI_UART_SUCCESS)
	   {
		   REPORT_ERROR("Could not do a read 0x%08X \n", eResult);
		   return FAILED;
	   }

	   // Store the incoming char into Rx Command Buffer;
	   // Unless it is a carriage return which specifies command gathering
	   // is done...
	   if(RxBuffer[0] != '\r')
	   {

		   char c = toupper((int)completedBuffer[0]);
		   memcpy((void *)&RxCommandBuffer[idxRx], (void *)&c, 1);
	   }

	   idxRx ++; // increment to the next storable address

		/* If return character is pressed, write back \n character along with \r */
		if(RxBuffer[0] == '\r')
		{
			RxBuffer[1] = '\n';

			/* Transmit two characters in this case */
			nTxSize     = 2u;

		   if (strcmp((const char *)RxCommandBuffer, "LED ON") == 0)
		   {
			   TermAction = LED_ON;
			   pBlinker->LedOn = true;
		   }
		   else if (strcmp((const char *)RxCommandBuffer, "LED OFF") == 0)
		   {
			   TermAction = LED_OFF;
			   pBlinker->LedOn = false;
			   bToggle = true;
			   bOnce = true;
		   }
		   else if (strcmp((const char *)RxCommandBuffer, "AUD ON") == 0)
		   {
			   TermAction = AUD_ON;
			   pBlinker->AudioOn = true;
			   pBlinker->LedOn = true;
			   pBlinker->LedBlinkRate = DEFAULT_BLINK_RATE;
			   pBlinker->FilterOn = false; // turn off filter
			   // Default led states
			   bToggle = true;
			   bOnce = true;
			   // Initialize our audio drivers
			   initialize_drivers(pBlinker->FilterOn);
		   }
		   else if (strcmp((const char *)RxCommandBuffer, "AUD OFF") == 0)
		   {
			   TermAction = AUD_OFF;
			   pBlinker->LedOn = false;
			   pBlinker->AudioOn = false;
			   pBlinker->FilterOn = false; // turn off filter
			   bToggle = true;
			   bOnce = true;
		   }
		   else if (strcmp((const char *)RxCommandBuffer, "FILT ON") == 0)
		   {
			   TermAction = FILT_ON;
			   pBlinker->LedOn = true;
			   pBlinker->LedBlinkRate = FILTER_BLINK_RATE;
			   pBlinker->FilterOn = true;
			   // Default led states
			   bToggle = true;
			   bOnce = true;
			   // Initialize our audio drivers
		       initialize_drivers(pBlinker->FilterOn);
		   }
		   else if (strcmp((const char *)RxCommandBuffer, "FILT OFF") == 0)
		   {
			   TermAction = FILT_OFF;
			   pBlinker->LedOn = false;
			   pBlinker->LedBlinkRate = DEFAULT_BLINK_RATE;
			   pBlinker->FilterOn = false;
			   initialize_drivers(pBlinker->FilterOn);
			   bToggle = true;
			   bOnce = true;
		   }
		   else if (strstr((const char *)RxCommandBuffer, "RATE") != NULL)
		   {

			   char rate[50]= {0};
			   char * from = ((char *)RxCommandBuffer) + 5;
			   uint8_t len = strlen((char *)RxCommandBuffer) - 5;
			   memcpy((void *)&rate, (void *)from, len);
			   uint32_t irate = atoi(rate);
			   // lets assign it if console input blink rate in the right ballpark
			   if (irate > MIN_LED_BLINK_RATE && irate <= MAX_LED_BLINK_RATE)
				   pBlinker->LedBlinkRate = irate;
			   else // otherwise, set it to default blink rate...
				   pBlinker->LedBlinkRate = DEFAULT_BLINK_RATE;

			   TermAction = LED_BLINK_RATE;
		   }
		   else if (strcmp((const char *)RxCommandBuffer, "TERM") == 0)
		   {
			   TermAction = TERM;
			   pBlinker->LedOn = false;
			   pBlinker->AudioOn = false;
			   pBlinker->FilterOn = false; // turn off filter
			   WelcomeMessage();
		   }

		   else if (strcmp((const char *)RxCommandBuffer, "QUIT") == 0)
		   {
			   TermAction = QUIT;
			   pBlinker->Exit = true; // indicate we wish to exit the entire processing scheme
			   bStopFlag = true;

		   }

		   // Reset the index and clear the commands buffer after carriage return

		   memset(&RxCommandBuffer, 0, sizeof(RxCommandBuffer));
		   idxRx = 0;
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

		memset((void *)&RxBuffer, 0, sizeof (RxBuffer)); // Clear out the output buffer

		// Give some verbosity indicating what we are doing
		// so they know our simple commands have succeeded...
		// Our verbose modes are:
		// UNDECIDED = -1,LED_ON, LED_OFF, LED_BLINK_RATE, QUIT
		switch (TermAction)
		{
			case UNDECIDED:
				// Take no action here and simply exit if bogus command at terminal
				break;
			case LED_ON:
				DEBUG(1,"[LED ON , RATE=%i]" _NL_ "", pBlinker->LedBlinkRate);
				break;
			case LED_OFF:
				DEBUG(1,"[LED OFF]" _NL_ "");
				break;
			case AUD_ON:
				DEBUG(1,"[AUD ON]" _NL_ "");
				DEBUG(1,"[FILT OFF]" _NL_ "");
				DEBUG(1,"[LED ON , RATE=%i]" _NL_ "", pBlinker->LedBlinkRate);
				break;
			case AUD_OFF:
				DEBUG(1,"[AUD OFF]" _NL_ "");
				DEBUG(1,"[FILT OFF]" _NL_ "");
				DEBUG(1,"[LED OFF]" _NL_ "");
				break;
			case FILT_ON:
				DEBUG(1,"[FILT ON]" _NL_ "");
				DEBUG(1,"[LED ON , RATE=%i]" _NL_ "", pBlinker->LedBlinkRate);
				break;
			case FILT_OFF:
				DEBUG(1,"[FILT OFF]" _NL_ "");
				DEBUG(1,"[LED OFF]" _NL_ "");
				break;
			case LED_BLINK_RATE:
				DEBUG(1,"[LED BLINK RATE=%i]" _NL_ "", pBlinker->LedBlinkRate);
				break;
			case TERM:
				DEBUG(1,"[TERM]" _NL_ "");
				break;
			case QUIT:
				DEBUG(1,"[QUITING - BYE!!!]" _NL_ "");
				break;
			default:
				break;
				// Ignore - We should never get here...
		}

		// Must do this so that verbose action does not fire twice!
		TermAction = UNDECIDED;
	 }

	// wait for the last printed command to expire
	for (int i = 0; i < 10000000; i ++)
		asm("nop;");

	/* Close the UART */
	if((eResult = adi_uart_Close(ghUART)) != ADI_UART_SUCCESS)
	{
		REPORT_ERROR("Could not close UART driver 0x%08X \n", eResult);
		return FAILED;
	}
	printf("QUIT command invoked. Closed UART connection! Bye... \n");
	return false; // means exit
}


static inline void FlashLEDIndicator(void)
{

	if (pBlinker->LedOn == true)
	{

		blinkCounter --;

		if (blinkCounter)
		{
			if (bToggle == true)
			{
				if (bOnce)
				{
					ledon();
					bOnce = !bOnce;
				}
			}
			else
			{
				if (!bOnce)
				{
					ledoff();
					bOnce = !bOnce;
				}
			}
			return;
		}

		bToggle = !bToggle;
		blinkCounter = pBlinker->LedBlinkRate;

	}
	else
	{
		if (bOnce)
		{
			ledoff();
			bOnce = !bOnce;
		}
	}
}


#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */
