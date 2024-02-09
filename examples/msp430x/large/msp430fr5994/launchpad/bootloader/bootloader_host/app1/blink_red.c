#include "tpl_os.h"
#include "msp430.h"
#include "leds.h"
#include <stdint.h>

extern uint16_t  PassWd_boot;
/*! Magic Key sent by Application to force boot mode */
#define BSL_PASSWORD        (0xC0DE)

#define APP_Task_blink_START_SEC_CODE
#include "tpl_memmap.h"


FUNC(int, OS_APPL_CODE) main(void)
{
	ledInit();
	/* set GPIO P5.6 (button S1) as an input, with internal pull-up */
	P5DIR &= ~(1<<6); /* input                                      */
	P5REN |= 1<<6;    /* pull-up/down resistor enable               */
	P5OUT |= 1<<6;    /* pull-up                                    */
	P5IES |= 1<<6;    /* high to low interrupt edge selection       */
	P5IE  |= 1<<6;    /* interrupt enabled for the button           */

	/* set GPIO P5.5 (button S2) as an input, with internal pull-up */
	P5DIR &= ~(1<<5); /* input                                      */
	P5REN |= 1<<5;    /* pull-up/down resistor enable               */
	P5OUT |= 1<<5;    /* pull-up                                    */
	P5IES |= 1<<5;    /* high to low interrupt edge selection       */
	P5IE  |= 1<<5;    /* interrupt enabled for the button           */

	StartOS(OSDEFAULTAPPMODE);
	return 0;
}

TASK(blink)
{
	ledToggle(LED1);
	TerminateTask();
}

#define APP_Task_blink_STOP_SEC_CODE
#include "tpl_memmap.h"

/*-----------------------------------------------------------------------------
 * ISR button
 */
#define APP_ISR_buttonS1_START_SEC_CODE
#include "tpl_memmap.h"

ISR(buttonS1)
{
	P1OUT ^= BIT1; //toggle green led
}

#define APP_ISR_buttonS1_STOP_SEC_CODE
#include "tpl_memmap.h"

/*-----------------------------------------------------------------------------
 * ISR button
 */
#define APP_ISR_buttonS2_START_SEC_CODE
#include "tpl_memmap.h"

ISR(buttonS2)
{
    PassWd_boot = BSL_PASSWORD;      // Send password
    __disable_interrupt();      // Disable all interrupts
    // Force a Software BOR
    PMMCTL0 = PMMPW | PMMSWBOR;
	// Not suppose to go here
    while (1);
}

#define APP_ISR_buttonS2_STOP_SEC_CODE
#include "tpl_memmap.h"

