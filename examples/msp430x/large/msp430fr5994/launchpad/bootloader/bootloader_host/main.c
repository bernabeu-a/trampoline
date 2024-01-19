#include "msp430.h"
#include <stdint.h>
#include <stdbool.h>

#include "lora/lora.h"

#include "blink_app1_red.c"

static uint16_t CRC_Addr = 0x4000;
static uint16_t App_StartAddress = 0x4003;
static uint16_t App_EndAddress = 0xEFFF;
static uint32_t App_StartAddress_Upper = 0x10000;
static uint32_t App_EndAddress_Upper = 0x43FFF;

static uint16_t CRC_App1;

static uint32_t Calc_App_CRC(uint32_t * Addr_array, uint32_t *Size_array, uint8_t ** DataPtr_Array, uint8_t num_arrays);

int main(void){
    /* Stop the watchdog timer */
    WDTCTL = WDTPW | WDTHOLD;

    /* Set clock */
    CSCTL0 = CSKEY;                     /* Get access to CSCTLx regs.           */

    CSCTL2 &= ~SELM_7;                  /* Set DCO as MCLK (clear field..       */
    CSCTL2 |= SELM__DCOCLK;             /* .. and set correct DCO value)        */

    CSCTL3 &= ~(DIVM0 | DIVM1 | DIVM2); /*  clear DIVM Field (source divider)   */
    CSCTL1 = DCOFSEL_0;                 /* 1 MHz                                */
    FRCTL0 = FWPW | NWAITS_0;           /* no wait state required               */

    /* First: set SMCLK to DCO */
    CSCTL0 = CSKEY;                     /* Get access to CSCTLx regs */
    CSCTL2 &= ~SELS_7;                  /* Set DCO as SMCLK (clear field..      */
    CSCTL2 |= SELS__DCOCLK;             /* .. and set correct DCO value)        */
    /* then remove the DIVS division */
    CSCTL3 &= ~(DIVS0_L | DIVS1_L | DIVS2_L);

    /* Compute CRC of application */
    CRC_App1 = Calc_App_CRC((uint32_t *)&blink_app1_red_Addr[0],
                            (uint32_t *)&blink_app1_red_Size[0],
                            (uint8_t **)&blink_app1_red_Ptr[0],
                            sizeof(blink_app1_red_Addr)/ sizeof (blink_app1_red_Addr[0]));

    /* LoRa init */
    LoRa_init();

    /* */
    uint16_t i;
    uint8_t *ptr_array = blink_app1_red_0;
    for(i=0; i<blink_app1_red_Size[0];i++){
//        send((uint8_t *)blink_app1_red_0[i], 1);
        send(ptr_array++, 1);
    }
    // while (1){
        // send(VBOOT_ENTRY_CMD, 1);
    // }
}

/* Calculate the CRC of the application*/
static uint32_t Calc_App_CRC(uint32_t * Addr_array, uint32_t *Size_array, uint8_t ** DataPtr_Array, uint8_t num_arrays)
{
    uint16_t addr;
    uint8_t i;

    CRCINIRES = 0xFFFF;

    // Calculate CRC for the whole Application address range
    for (addr=App_StartAddress; addr <= App_EndAddress; addr++)
    {
        for (i = 0; i < num_arrays; i ++)
        {
            // Check if address is defined by application image
            if ( (addr >= Addr_array[i]) &&
                 (addr < (Addr_array[i] + Size_array[i])) )
            {
                // If address is defined, add it to the CRC
                CRCDIRB_L = DataPtr_Array[i][addr-Addr_array[i]];
                break;
            }
        }
        if (i==num_arrays)
        {
            // If not, simply add 0xFF
            CRCDIRB_L = 0xFF;
        }
    }
    // CRC includes the upper Application address range
    uint32_t addr20;
    for (addr20=App_StartAddress_Upper; addr20 <= App_EndAddress_Upper; addr20++)
    {
        for (i = 0; i < num_arrays; i ++)
        {
            // Check if address is defined by application image
            if ( (addr20 >= Addr_array[i]) &&
                 (addr20 < (Addr_array[i] + Size_array[i])) )
            {
                // If address is defined, add it to the CRC
                CRCDIRB_L = DataPtr_Array[i][addr20-Addr_array[i]];
                break;
            }
        }
        if (i==num_arrays)
        {
            // If not, simply add 0xFF
            CRCDIRB_L = 0xFF;
        }
    }
    return CRCINIRES;
}
