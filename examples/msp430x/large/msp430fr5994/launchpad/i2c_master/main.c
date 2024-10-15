#include <stdint.h>
#include "tpl_os.h"
#include "msp430.h"

/*-----------------------------------------------------------------------------
 * Hardware init and main
 */

volatile unsigned char RXData;

#define OS_START_SEC_CODE
#include "tpl_memmap.h"

FUNC(int, OS_APPL_CODE) main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    // Configure GPIO
    P1OUT &= ~BIT0;                         // Clear P1.0 output latch
    P1DIR |= BIT0;                          // For LED
    P7SEL0 |= BIT0 | BIT1;
    P7SEL1 &= ~(BIT0 | BIT1);
    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    // Configure USCI_B2 for I2C mode
    UCB2CTLW0 |= UCSWRST;                   // Software reset enabled
    UCB2CTLW0 |= UCMODE_3 | UCMST | UCSYNC; // I2C mode, Master mode, sync
    UCB2CTLW1 |= UCASTP_2;                  // Automatic stop generated
                                            // after UCB2TBCNT is reached
    UCB2BRW = 0x0008;                       // baudrate = SMCLK / 8
    UCB2TBCNT = 0x0005;                     // number of bytes to be received
    UCB2I2CSA = 0x0048;                     // Slave address
    UCB2CTL1 &= ~UCSWRST;
    UCB2IE |= UCRXIE | UCNACKIE | UCBCNTIE;



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
        while (UCB2CTL1 & UCTXSTP);         // Ensure stop condition got sent
        UCB2CTL1 |= UCTXSTT;                // I2C start condition
        __bis_SR_register(LPM0_bits | GIE); // Enter LPM0 w/ interrupt
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

}

#define APP_ISR_USCI_TX_STOP_SEC_CODE
#include "tpl_memmap.h"


/*-----------------------------------------------------------------------------
 * ISR I2C RX
 */
#define APP_ISR_USCI_RX_START_SEC_CODE
#include "tpl_memmap.h"

ISR(USCI_RX)
{
	RXData = UCB2RXBUF;             // Get RX data
}

#define APP_ISR_USCI_RX_STOP_SEC_CODE
#include "tpl_memmap.h"

/*-----------------------------------------------------------------------------
 * ISR I2C UCBNT
 */
#define APP_ISR_USCI_UCBCNT_START_SEC_CODE
#include "tpl_memmap.h"

ISR(USCI_UCBCNT)
{
    P1OUT ^= BIT0;                  // Toggle LED on P1.0
}

#define APP_ISR_USCI_UCBCNT_STOP_SEC_CODE
#include "tpl_memmap.h"


/*-----------------------------------------------------------------------------
 * ISR I2C NACK
 */
#define APP_ISR_USCI_NACK_START_SEC_CODE
#include "tpl_memmap.h"

ISR(USCI_NACK)
{
    UCB2CTL1 |= UCTXSTT;            // I2C start condition
}

#define APP_ISR_USCI_NACK_STOP_SEC_CODE
#include "tpl_memmap.h"
