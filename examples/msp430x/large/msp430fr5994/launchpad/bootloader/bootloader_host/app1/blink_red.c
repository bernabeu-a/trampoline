#include "tpl_os.h"
#include "msp430.h"
#include "leds.h"

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