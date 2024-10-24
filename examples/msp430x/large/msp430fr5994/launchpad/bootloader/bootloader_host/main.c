#include "msp430.h"
#include <stdint.h>
#include <stdbool.h>

#include "lora/lora.h"

// #include "blink_app1_red.c"
#include "./app1/app1.c"

#define TX_BUF_SIZE 224
#define RESPONSE_OK 0x00

static uint16_t CRC_Addr = 0x4000;
static uint16_t App_StartAddress = 0x4003;
static uint16_t App_EndAddress = 0xCFFF;
static uint32_t App_StartAddress_Upper = 0x10000;
static uint32_t App_EndAddress_Upper = 0x247FF;

static uint32_t CRC_App1 = 0;
unsigned char txBuffer[TX_BUF_SIZE];

typedef struct sBSLPayload
{
    uint8_t ui8Command;
    uint8_t ui8Addr_L;
    uint8_t ui8Addr_M;
    uint8_t ui8Addr_H;
    const uint8_t* ui8pData;
} tBSLPayload;

typedef struct sBSLPacket
{
    uint8_t ui8Header;
    uint8_t ui8Length;
    tBSLPayload tPayload;
    uint16_t ui16Checksum;
} tBSLPacket;

uint32_t Calc_App_CRC(uint32_t * Addr_array, uint32_t *Size_array, uint8_t ** DataPtr_Array, uint8_t num_arrays);
uint16_t crc16MakeBitwise(tBSLPacket tPacket);

uint8_t BSL_programMemorySegment(uint32_t addr, const uint8_t* data, uint32_t len){
    uint16_t xferlen;
    uint8_t res;
    uint8_t crch, crcl;
    uint16_t ii;

    while(len > 0){
        P1OUT ^= BIT0;
        if(len > 216){
            xferlen = 216;
        }
        else{
            xferlen = len;
        }

        tBSLPacket tPacket;
        /* Command */
        tPacket.tPayload.ui8Command = 0xAA;
        /* Data */
        tPacket.tPayload.ui8pData = data;
        /* Length */
        tPacket.ui8Length = xferlen + 4;
        /* Address*/
        tPacket.tPayload.ui8Addr_H = ((uint8_t) (addr >> 16));
        tPacket.tPayload.ui8Addr_M = ((uint8_t) (addr >> 8));
        tPacket.tPayload.ui8Addr_L = ((uint8_t) (addr & 0xFF));
        tPacket.ui16Checksum = crc16MakeBitwise(tPacket);

        /* Now put data in txBuffer */
        txBuffer[0] = 0x80;                                         /* Header */
        txBuffer[1] = tPacket.ui8Length;                            /* Length */
        txBuffer[2] = tPacket.tPayload.ui8Command;                           /* Command */
        if(tPacket.ui8Length > 1){
            txBuffer[3] = tPacket.tPayload.ui8Addr_L;               /* Address */
            txBuffer[4] = tPacket.tPayload.ui8Addr_M;
            txBuffer[5] = tPacket.tPayload.ui8Addr_H;
            for (ii = 0; ii < (tPacket.ui8Length - 4); ii++) {
                txBuffer[6+ii] = tPacket.tPayload.ui8pData[ii];     /* Data */
            }
            crcl = (uint8_t) (tPacket.ui16Checksum & 0xFF);         /* crc */
            txBuffer[6+ii] = crcl;

            crch = (uint8_t) (tPacket.ui16Checksum >> 8);
            txBuffer[7+ii] = crch;
        }
        else{
			crcl = (uint8_t) (tPacket.ui16Checksum & 0xFF);
			txBuffer[3] = crcl;

			crch = (uint8_t) (tPacket.ui16Checksum >> 8);
			txBuffer[4] = crch;
		}

        uint16_t wait;
        for(wait=0; wait<0xFFFF; wait++);
        for(wait=0; wait<0xFFFF; wait++);
        /* Send Data */
        send(txBuffer, TX_BUF_SIZE);
        /* Now wait for answer */
        uint8_t rx_length = TX_BUF_SIZE;
        recv(txBuffer, rx_length);

        while(txBuffer[0] != RESPONSE_OK){
            P1OUT ^= BIT1;
            // Send again
            tBSLPacket tPacket;
            /* Command */
            tPacket.tPayload.ui8Command = 0xAA;
            /* Data */
            tPacket.tPayload.ui8pData = data;
            /* Length */
            tPacket.ui8Length = xferlen + 4;
            /* Address*/
            tPacket.tPayload.ui8Addr_H = ((uint8_t) (addr >> 16));
            tPacket.tPayload.ui8Addr_M = ((uint8_t) (addr >> 8));
            tPacket.tPayload.ui8Addr_L = ((uint8_t) (addr & 0xFF));
            tPacket.ui16Checksum = crc16MakeBitwise(tPacket);

            /* Now put data in txBuffer */
            txBuffer[0] = 0x80;                                         /* Header */
            txBuffer[1] = tPacket.ui8Length;                            /* Length */
            txBuffer[2] = tPacket.tPayload.ui8Command;                           /* Command */
            if(tPacket.ui8Length > 1){
                txBuffer[3] = tPacket.tPayload.ui8Addr_L;               /* Address */
                txBuffer[4] = tPacket.tPayload.ui8Addr_M;
                txBuffer[5] = tPacket.tPayload.ui8Addr_H;
                for (ii = 0; ii < (tPacket.ui8Length - 4); ii++) {
                    txBuffer[6+ii] = tPacket.tPayload.ui8pData[ii];     /* Data */
                }
                crcl = (uint8_t) (tPacket.ui16Checksum & 0xFF);         /* crc */
                txBuffer[6+ii] = crcl;

                crch = (uint8_t) (tPacket.ui16Checksum >> 8);
                txBuffer[7+ii] = crch;
            }
            else{
                crcl = (uint8_t) (tPacket.ui16Checksum & 0xFF);
                txBuffer[3] = crcl;

                crch = (uint8_t) (tPacket.ui16Checksum >> 8);
                txBuffer[4] = crch;
            }
            for(wait=0; wait<0xFFFF; wait++);
            for(wait=0; wait<0xFFFF; wait++);
            /* Send Data */
            send(txBuffer, TX_BUF_SIZE);            
            /* Now wait for answer */
            uint8_t rx_length = TX_BUF_SIZE;
            recv(txBuffer, rx_length);
            // break;
        }

        len -= xferlen;
        addr += xferlen;
        data += xferlen;
    }
}

uint8_t sendCommand(uint8_t cmd)
{
    uint8_t crch, crcl;
    uint16_t ii;

    tBSLPacket tPacket;
    tPacket.ui8Header = 0x80;
    tPacket.tPayload.ui8Command = cmd;
    tPacket.ui8Length = 1;
    tPacket.ui16Checksum = crc16MakeBitwise(tPacket);
    /* Now put data in txBuffer */
    txBuffer[0] = 0x80;                                         /* Header */
    txBuffer[1] = tPacket.ui8Length;                            /* Length */
    txBuffer[2] = tPacket.tPayload.ui8Command;                           /* Command */
    if(tPacket.ui8Length > 1){
        txBuffer[3] = tPacket.tPayload.ui8Addr_L;               /* Address */
        txBuffer[4] = tPacket.tPayload.ui8Addr_M;
        txBuffer[5] = tPacket.tPayload.ui8Addr_H;
        for (ii = 0; ii < (tPacket.ui8Length - 4); ii++) {
            txBuffer[6+ii] = tPacket.tPayload.ui8pData[ii];     /* Data */
        }
        crcl = (uint8_t) (tPacket.ui16Checksum & 0xFF);         /* crc */
        txBuffer[6+ii] = crcl;

        crch = (uint8_t) (tPacket.ui16Checksum >> 8);
        txBuffer[7+ii] = crch;
    }
    else{
        crcl = (uint8_t) (tPacket.ui16Checksum & 0xFF);
        txBuffer[3] = crcl;

        crch = (uint8_t) (tPacket.ui16Checksum >> 8);
        txBuffer[4] = crch;
    }
    /* Send Data */
    uint16_t wait;
    for(wait=0; wait<0xFFFF; wait++);
    for(wait=0; wait<0xFFFF; wait++);
    send(txBuffer, TX_BUF_SIZE);
    return 0;
}

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

    P1DIR |= BIT0+BIT1;
    P1OUT &= ~(BIT0+BIT1);

    /* Compute CRC of application */
    // CRC_App1 = Calc_App_CRC((uint32_t *)&blink_app1_red_Addr[0],
    //                         (uint32_t *)&blink_app1_red_Size[0],
    //                         (uint8_t **)&blink_app1_red_Ptr[0],
    //                         sizeof(blink_app1_red_Addr)/ sizeof (blink_app1_red_Addr[0]));
    CRC_App1 = Calc_App_CRC( (uint32_t *)&app1_Addr[0],
                              (uint32_t *)&app1_Size[0],
                              (uint8_t **)&app1_Ptr[0],
                              sizeof(app1_Addr)/ sizeof (app1_Addr[0]));
    /* LoRa init */
    LoRa_init();

    uint8_t section;
    uint8_t res;

    for (section = 0; section < (sizeof(app1_Addr)/ sizeof (app1_Addr[0])) ; section++){
        /* Sending the segments*/
        res = BSL_programMemorySegment(app1_Addr[section], app1_Ptr[section], app1_Size[section]);       
    }
    P1OUT |= BIT1;
    res = BSL_programMemorySegment(CRC_Addr, (uint8_t *) &CRC_App1, 2);

    /* Jumping to user code */
    res = sendCommand(0x1C);
    P1OUT &= ~BIT1;
//     /* */
//     uint16_t i;
//     uint8_t *ptr_array = blink_app1_red_0;
//     for(i=0; i<blink_app1_red_Size[0];i++){
// //        send((uint8_t *)blink_app1_red_0[i], 1);
//         send(ptr_array++, 1);
//     }
//     // while (1){
//         // send(VBOOT_ENTRY_CMD, 1);
//     // }
}

/* Calculate the CRC of the application*/
uint32_t Calc_App_CRC(uint32_t * Addr_array, uint32_t *Size_array, uint8_t ** DataPtr_Array, uint8_t num_arrays)
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

/* Calculate the CRC of a packet */

uint16_t crc16MakeBitwise(tBSLPacket tPacket)
{
    uint16_t i;

    CRCINIRES = 0xFFFF;

    uint8_t* pmsg = (uint8_t*)&tPacket.tPayload;

    uint16_t msg_size = tPacket.ui8Length;

    for(i = 0 ; i < msg_size ; i ++)
    {
        if(i < 4)
        	CRCDIRB_L = pmsg[i];
        else if(i == 4)
        {
            pmsg = (uint8_t*)tPacket.tPayload.ui8pData;
            CRCDIRB_L = pmsg[i-4];
        }
        else
        {
        	CRCDIRB_L = pmsg[i-4];
        }

    }

    return CRCINIRES;
}