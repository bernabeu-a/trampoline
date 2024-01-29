#include "msp430.h"
#include <stdbool.h>
#include <stdint.h>

#include "lora/lora.h"

// #define __fram __attribute__((section(".persistent")))
// #define blink_app1_red_SIZE   10005

#define TX_BUF_SIZE 24
#define RESPONSE_OK 0x00
#define RESPONSE_ERROR 0x99
#define COMM_ERROR 0x99
#define COMM_OK 0x00
// __fram uint8_t blink_app1_red_0[blink_app1_red_SIZE] = {0};

/* Global Variable */
volatile unsigned char txBuffer[TX_BUF_SIZE];
uint8_t RxPacket[TX_BUF_SIZE];

uint8_t CommStatus;
static uint8_t Len;
static uint8_t counter;
uint8_t TxByte;

uint16_t crc16MakeBitwise(uint8_t *pmsg, uint32_t msg_size);

// __asm__ volatile(
//     "\t.global _reset_vector                \n"
//     "\t.section .init, \"ax\", @progbits    \n"
//     "\t.type _start_bootloader, %function   \n"
//     "\t _start_bootloader:                  \n"
//     "calla #_start_bootloader               \n"
// );

void __attribute__ ((interrupt(RESET_VECTOR))) _start_bootloader (void){
    main_boot();

}

void rxProcess(uint8_t data){
    static uint16_t Checksum;
    uint8_t ii;
    if(counter == 0){
        TxByte = 0x99;
        // Byte 0 Header
        // Check Header
        if (data != 0x80){
            // Incorrect Header --> Error
            CommStatus |= COMM_ERROR;
            TxByte = 0x99;
        }
    }

    else if(counter == 1){
        // Byte 1 Len
        Len = data;
        if(data == 0){
            // Size 0 --> Error
            CommStatus |= COMM_ERROR;
            TxByte = 0x99;
        }
        else if (data > 20){
            // Size too big --> Error
            CommStatus |= COMM_ERROR;
            TxByte = 0x99;
        }
    }
    else if(counter < (Len+2)){
        // Payload data (optional Addr +  data)
        RxPacket[counter-2] = data;
    }
    else if(counter == (Len+2)){
        // Byte n-1 expected crc_l
        Checksum = data;
    }
    else if(counter == (Len+3)){
        // Byte n expected crc_h
        Checksum += (data<<8);
        // Now calculate crc and compare
        if(crc16MakeBitwise(RxPacket, Len) != Checksum){
            // Incorrect checksum --> Error
            CommStatus |= COMM_ERROR;
            TxByte = 0x99;
        }
        else{
            // Packet OK
            CommStatus |= COMM_OK;
            TxByte = 0x00;
        }
    }
    if(CommStatus & COMM_ERROR){
        // An error occured, send ack error
        counter = 0;
        CommStatus = 0;
        
        uint8_t rx_length = TX_BUF_SIZE;
        txBuffer[0] = TxByte;
        for (ii = 1; ii < rx_length; ii++) txBuffer[ii] = 0;
        send(txBuffer, rx_length);
    }
    else{
        counter++;
    }
}

uint8_t WriteByte(uint32_t addr, uint8_t data){
    __data20_write_char(addr, data);
    return RESPONSE_OK;
}

void rxIntepreter(uint8_t *RxData, uint8_t RxLen, uint8_t *TxData){
    uint8_t ii;
    uint8_t i;    
    switch (RxData[0]){
        case 0xAA:

            for(i = 0; i < RxLen-4; i++){
                if(WriteByte((uint32_t)RxData[1]+((uint32_t)RxData[2]<<8)+(((uint32_t)RxData[3] & 0x0F)<<16), &RxData[4+i]) != RESPONSE_OK){
                    *TxData = RESPONSE_ERROR;
                }
            }
            *TxData = RESPONSE_OK;
            // Everything OK, send ack OK
            counter = 0;
            CommStatus = 0;
            uint8_t rx_length = TX_BUF_SIZE;
            txBuffer[0] = TxByte;
            for (ii = 1; ii < rx_length; ii++) txBuffer[ii] = 0;
            send(txBuffer, rx_length);
    }
}

int main_boot( void ) {
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

    /* */
    P1DIR |= BIT0 | BIT1;
    P1OUT &= ~(BIT0+BIT1);
    /* LoRa init */
    LoRa_init();


    while(1){
        uint16_t ii;
        uint8_t rx_length = TX_BUF_SIZE;
        /* Receive Data */
        recv(txBuffer, rx_length);
        /* Now process it */
        if(txBuffer[0] == 0x80 && txBuffer[1] > 0 && txBuffer[1] <= 20){
            if (txBuffer[1] == 1){
                rx_length = 5;                            /* Only crc */
            }
            else{
                rx_length = txBuffer[1] + 4;
            } 
            for (ii = 0; ii < rx_length; ii++){
                rxProcess(txBuffer[ii]);
            }
        }
        /* Now interpret it */
        if(CommStatus == COMM_OK){
                rxIntepreter(RxPacket, Len, &TxByte);
                P1OUT ^= BIT1;
        }

    /**/
    // uint16_t i;
    // uint8_t *ptr_array = blink_app1_red_0;
    
    // for(i=0; i<blink_app1_red_SIZE;i++){
    //     P1OUT ^= BIT1;
    //     recv(ptr_array++, 1);
    // }
    P1OUT |= BIT0;
    }
}


/* Calculate the CRC of a packet */

uint16_t crc16MakeBitwise(uint8_t *pmsg, uint32_t msg_size)
{
    uint32_t i;
    
    CRCINIRES = 0xFFFF;
    for(i = 0 ; i < msg_size ; i ++)
    {
        CRCDIRB_L = *pmsg++;
    }
    return(CRCINIRES);
}