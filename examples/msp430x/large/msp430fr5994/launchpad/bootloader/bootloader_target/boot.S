	
 #include "msp430.h"
    
    .section .init, "ax", @progbits
	.global _start_bootloader
	.type _start_bootloader, %function

_start_bootloader:
	movx.a #0xA500, &MPUCTL0 /* unlock MPU */
	movx.a #0xA500, &MPUCTL0 /* unlock MPU */
	/* watchdog support */
/* #if WITH_DISABLE_WDT */
	movx.a     #0x5a80, &WDTCTL
/* #else 
	movx.b	&WDTCTL, r5
	bisx	#0x05a08, r5
	movx	r5, &__wdt_clear_value  
#endif */

	/* disable interrupts at startup
	 * they will be enabled when the first task wil start (including idle task)
	*/
	dint
    nop
	/* init stack */
	mova     #__RAM_End-2, r1

	calla    #_start_bootloader_continue