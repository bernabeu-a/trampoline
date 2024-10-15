#include <stdint.h>
#include "tpl_os.h"
#include "msp430.h"

/*-----------------------------------------------------------------------------
 * Hardware init and main
 */

volatile unsigned char TXData;

#define OS_START_SEC_CODE
#include "tpl_memmap.h"

FUNC(int, OS_APPL_CODE) main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    // Configure GPIO
    P7SEL0 |= BIT0 | BIT1;
    P7SEL1 &= ~(BIT0 | BIT1);
    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    // Configure USCI_B2 for I2C mode
    UCB2CTLW0 |= UCSWRST;                   // Software reset enabled
    UCB2CTLW0 |= UCMODE_3 | UCSYNC;         // I2C mode, sync
    UCB2I2COA0 = 0x48 | UCOAEN;             // own address is 0x48 + enable
    UCB2CTL1 &= ~UCSWRST;
    UCB2IE |= UCTXIE0 | UCSTPIE;            // transmit,stop interrupt enable



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
        __bis_SR_register(LPM0_bits | GIE);     // Enter LPM0 w/ interrupts
        __no_operation();
    }
	TerminateTask();
}

#define APP_Task_blink_STOP_SEC_CODE
#include "tpl_memmap.h"

/*-----------------------------------------------------------------------------
 * ISR I2C TX
 */
#define APP_ISR_USCI_TX_START_SEC_CODE
#include "tpl_memmap.h"

ISR(USCI_TX)
{
    UCB2TXBUF = TXData++;
}

#define APP_ISR_USCI_TX_STOP_SEC_CODE
#include "tpl_memmap.h"

/*-----------------------------------------------------------------------------
 * ISR I2C UCSTP
 */
#define APP_ISR_USCI_UCSTP_START_SEC_CODE
#include "tpl_memmap.h"

ISR(USCI_UCSTP)
{
    TXData = 0;
    UCB2IFG &= ~UCSTPIFG;           // Clear stop condition int flag    
}

#define APP_ISR_USCI_UCSTP_STOP_SEC_CODE
#include "tpl_memmap.h"
