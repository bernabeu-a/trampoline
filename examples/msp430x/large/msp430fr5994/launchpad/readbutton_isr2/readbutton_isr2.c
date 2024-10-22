#include <stdint.h>
#include "tpl_os.h"
#include "msp430.h"

/*-----------------------------------------------------------------------------
 * Hardware init and main
 */
#define OS_START_SEC_CODE
#include "tpl_memmap.h"

/*
 * init some I/Os of the board:
 * - red   LED1  on P1.0
 * - green LED2  on P1.1
 * - S1 user button on P5.6
 */
FUNC(void, OS_APPL_CODE) ioInit()
{
  /*
   * Disable the GPIO power-on default high-impedance mode
   * to activate previously configured port settings
   */
  PM5CTL0 &= ~LOCKLPM5;
  /* set GPIO P1.0 and 1.1 (red LED1 and green LED2) as an output */
  P1DIR |= 0x03;
  P1OUT &= ~(BIT0+BIT1);
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
}

FUNC(int, OS_APPL_CODE) main(void)
{
  ioInit();
  StartOS(OSDEFAULTAPPMODE);
  return 0;
}

#define OS_STOP_SEC_CODE
#include "tpl_memmap.h"

/*-----------------------------------------------------------------------------
 * task blink
 */
#define APP_Task_blink_START_SEC_CODE
#include "tpl_memmap.h"

TASK(blink)
{
  while(1){
    __bis_SR_register(LPM0_bits | GIE);
    P1OUT ^= BIT0;
  }
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
  
}

#define APP_ISR_buttonS2_STOP_SEC_CODE
#include "tpl_memmap.h"