#include "msp430.h"
#include <stdbool.h>
#include <stdint.h>

#include "lora/lora.h"

extern uint16_t _Appl_End;                  /* Application End Address */
extern uint32_t _Appl_Start2;
extern uint32_t _Appl_End2;
extern uint16_t _Appl_checksum;             /*! Application Checksum Address */
extern uint16_t _Appl_Vector_Start;         /*! Application Vector Table Start */
extern uint16_t _Appl_Reset_Vector;         /*! Application Reset vector */
extern uint16_t __Boot_VectorTable;         /*! Bootloader Vector Table Start */
extern uint16_t __Boot_Start;				/*! Bootloader Start */
extern uint16_t __RAM_End; 
/*! Application start address (from linker file) */
#define APP_START_ADDR          ((uint32_t )&_Appl_checksum)
/*! Application end address (from linker file) */
#define APP_END_ADDR            ((uint32_t )&_Appl_End)
/*! App2 area start address (from linker file) */
#define APP2_START_ADDR          ((uint32_t )&_Appl_Start2)
/*! App2 end address (from linker file) */
#define APP2_END_ADDR            ((uint32_t )&_Appl_End2)
/*! Application Vector Table */
#define APP_VECTOR_TABLE        ((uint32_t) &_Appl_Vector_Start)
/*! Application Reset Vector */
#define APP_RESET_VECTOR_ADDR   ((uint32_t) &_Appl_Reset_Vector)
#define BOOT_VECTOR_TABLE       ((uint32_t) &__Boot_VectorTable)
#define RAM_END               ((uint32_t) &__RAM_End)

// #define __fram __attribute__((section(".persistent")))
// #define blink_app1_red_SIZE   10005

#define TX_BUF_SIZE 24
#define RESPONSE_OK 0x00
#define RESPONSE_ERROR 0x99
#define COMM_ERROR 0x99
#define COMM_OK 0x00
#define RET_JUMP_TO_APP     2
#define RET_OK              0
#define RET_ERROR           1

// __fram uint8_t blink_app1_red_0[blink_app1_red_SIZE] = {0};

/* Global Variable */
volatile unsigned char txBuffer[TX_BUF_SIZE];
uint8_t RxPacket[TX_BUF_SIZE];

uint8_t CommStatus;
static uint8_t Len;
static uint8_t counter;
uint8_t TxByte;

uint16_t crc16MakeBitwise(uint8_t *pmsg, uint32_t msg_size);
uint32_t TI_MSPBoot_MI_GetPhysicalAddressFromVirtual(uint32_t addr);
static bool Verify_App_Down(void);
// __asm__ volatile(
//     "\t.global _reset_vector                \n"
//     "\t.section .init, \"ax\", @progbits    \n"
//     "\t.type _start_bootloader, %function   \n"
//     "\t _start_bootloader:                  \n"
//     "calla #main_boot                       \n"
// );

void _start_bootloader_continue(void){
    /* Init .bss section */
    extern unsigned __bssstart;
    extern unsigned __bssend;
    unsigned *p = &__bssstart;
    while (p != &__bssend) {
    *p = 0;
    p++;
    }

    // __datastart

    // unsigned *pSrc = src;
    // unsigned *pDest = dst;
    // unsigned *pDestMax = dst+size;
    // while (pDest != pDestMax) {
    // *pDest = *pSrc;
    // pDest++;
    // pSrc++;
    // }
    main_boot();
}

// void __attribute__ ((interrupt(RESET_VECTOR))) _start_bootloader (void){
//     // Des choses a faire avant le main_boot ? 
//     // init des variable etc ... ?
//     __set_SP_register((RAM_END-2));
//     // __asm__ volatile ("\t   MOVX.A\t   #RAM_END-2,SP");
//     main_boot();
// }

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
    if ((addr >= APP_VECTOR_TABLE) && (addr < APP_RESET_VECTOR_ADDR - 2))
    {
        // If address is an interrupt vector, copy directly to interrupt table
        addr = (addr - APP_VECTOR_TABLE) + BOOT_VECTOR_TABLE;
        __data20_write_char(addr, data);    // Write to interrupt table
    }
    else{
        __data20_write_char(addr, data);
    }
    return RESPONSE_OK;
}

uint8_t rxIntepreter(uint8_t *RxData, uint8_t RxLen, uint8_t *TxData){
    uint8_t ii;
    uint8_t i;    
    switch (RxData[0]){
        // Rx data command
        case 0xAA:
            for(i = 0; i < RxLen-4; i++){
                if(WriteByte(TI_MSPBoot_MI_GetPhysicalAddressFromVirtual((uint32_t)RxData[1]+((uint32_t)RxData[2]<<8)+(((uint32_t)RxData[3] & 0x0F)<<16)), &RxData[4+i]) != RESPONSE_OK){
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
            break;
        // Jump to APP command
        case 0x1C:
            return RET_JUMP_TO_APP;
    }
    return RET_OK;
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

    // Check if application recquired boot mode, and check if application is OK 
    // if so jump to reset vector application
    if((*(volatile uint16_t *)(&_Appl_Reset_Vector) != 0xFFFF)){  
        ((void (*)()) _Appl_Reset_Vector) ();
    }
    // Image invalid, check Download Area
    else if (Verify_App_Down()){
        // Erase App Area
        uint32_t addr;
        for (addr = APP_START_ADDR; addr <= APP_END_ADDR; addr+=2){
            __data20_write_short(addr, 0xFFFF);            
        }
		for (addr = APP2_START_ADDR; addr <= APP2_END_ADDR; addr+=2){
		    __data20_write_short(addr, 0xFFFF);
        }
        // Replace the Image in App Area by Download Area
        /* APP from Download area 1 */
        for (addr = APP_START_ADDR; addr <= APP_END_ADDR; addr++){
            WriteByte(addr, __data20_read_char(TI_MSPBoot_MI_GetPhysicalAddressFromVirtual(addr)));
            __no_operation();
        }
        /* APP from download area 2 */
        for (addr = APP2_START_ADDR; addr <= APP2_END_ADDR; addr++)
        {
            WriteByte(addr, __data20_read_char(TI_MSPBoot_MI_GetPhysicalAddressFromVirtual(addr)));
            __no_operation();
        }  
        // Validate image in App Area
        if (*(volatile uint16_t *)(&_Appl_Reset_Vector) != 0xFFFF){
            // Erase download 
            for (addr = APP_START_ADDR; addr <= APP_END_ADDR; addr+=2){
                __data20_write_short(TI_MSPBoot_MI_GetPhysicalAddressFromVirtual(addr), 0xFFFF);
            }
            for (addr = APP2_START_ADDR; addr <= APP2_END_ADDR; addr+=2){
                __data20_write_short(TI_MSPBoot_MI_GetPhysicalAddressFromVirtual(addr), 0xFFFF);
            }
            // Jump to App 
            ((void (*)()) _Appl_Reset_Vector) ();
        }
    }
    else{
        // Boot is forced from app, continue to while
    }

    while(1){
        uint16_t ii;
        uint8_t rx_length = TX_BUF_SIZE;
        uint8_t ret;
        /* Receive Data */
        P1OUT ^= BIT0;
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
            ret = rxIntepreter(RxPacket, Len, &TxByte);
        }

        if (ret == RET_JUMP_TO_APP)
        {
            P1OUT ^= BIT1;
            // If Packet indicates a jump to App
            TI_MSPBoot_AppMgr_JumpToApp();
        }
    /**/
    // uint16_t i;
    // uint8_t *ptr_array = blink_app1_red_0;
    
    // for(i=0; i<blink_app1_red_SIZE;i++){
    //     P1OUT ^= BIT1;
    //     recv(ptr_array++, 1);
    // }
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

/* Verify Application in Download area */

static bool Verify_App_Down(void){
    extern uint32_t _Download_start_memory;
    extern uint32_t _Download_checksum;
    extern uint32_t _Download_CRC_size;
    
    static uint16_t result = 0;
    uint32_t i;
    uint8_t* data_ptr;

    CRCINIRES = 0xFFFF;                                                                 /* Init seed */
    data_ptr = (uint8_t*) &_Download_start_memory;
    for(i = 0 ; i < (uint32_t) &_Download_CRC_size ; i++) CRCDIRB_L = *data_ptr++;     /* Add to crc */

    result = CRCINIRES;
    if (result != __data20_read_short((unsigned long)&_Download_checksum)) return false;
    else return true;
}

/******************************************************************************
 *
 * @brief   Convert a virtual address of application to a physical address in
 *          download area
 *
 * @param  addr     Address in appplication memory
 *
 * @return  Physical address in download area
 *
 * @note: the address must be in application area, this function doesn't validate
 *          the input
 *****************************************************************************/
uint32_t TI_MSPBoot_MI_GetPhysicalAddressFromVirtual(uint32_t addr)
{
    volatile uint32_t ret;
    volatile uint32_t address;
    extern uint32_t _Download_offset_size;			/*! Download Offset Size */
    extern uint32_t _Download_offset1;				/*! Download Offset 1 */
    extern uint32_t _Download_offset2;				/*! Download Offset 2 */

    address = addr;

    /* 
     * There is 2 download areas
     * For addresses from 0x4000 (start of app1) to 0xdfff (end of app1) --> addr + 0x20800
     * so download addr is from 0x24800 to 0x2e7ff
     * For addresses from 0x10000 (start of app2) to 0x247ff (end of app2) --> addr + (0xe000 - 0x4000 + 0x24800 - 0x10000)
     * so download addr is from 0x2e800 to 0x42fff
     */

    // If the address-offset fits in 1st app area
    if (address <=  (uint32_t )&_Download_offset_size)
	{
		ret = (addr + (uint32_t )&_Download_offset1);
	}
	else    // Else, place it in the 2nd download area
	{
		ret = (addr + (uint32_t )&_Download_offset2);
	}

    return ret;
}

void TI_MSPBoot_MI_ReplaceApp(void)
{
    volatile uint32_t addr;

    for (addr = APP_START_ADDR; addr <= APP_END_ADDR; addr++)
    {
        WriteByte(addr, __data20_read_char(TI_MSPBoot_MI_GetPhysicalAddressFromVirtual(addr)));
        __no_operation();
    }

    if(APP2_START_ADDR == 0x10000)
    {
        for (addr = APP2_START_ADDR; addr <= APP2_END_ADDR; addr++)
        {
        	WriteByte(addr, __data20_read_char(TI_MSPBoot_MI_GetPhysicalAddressFromVirtual(addr)));
        	__no_operation();
        }
    }
}

void TI_MSPBoot_AppMgr_JumpToApp(void){
    if(Verify_App_Down()){
        // Erase current app
        uint32_t addr;
        for (addr = APP_START_ADDR; addr <= APP_END_ADDR; addr+=2){
            __data20_write_short(addr, 0xFFFF);            
        }
		for (addr = APP2_START_ADDR; addr <= APP2_END_ADDR; addr+=2){
		    __data20_write_short(addr, 0xFFFF);
        }
        // Switch Download to app
        /* APP from Download area 1 */
        for (addr = APP_START_ADDR; addr <= APP_END_ADDR; addr++){
            WriteByte(addr, __data20_read_char(TI_MSPBoot_MI_GetPhysicalAddressFromVirtual(addr)));
            __no_operation();
        }
        /* APP from download area 2 */
        for (addr = APP2_START_ADDR; addr <= APP2_END_ADDR; addr++)
        {
            WriteByte(addr, __data20_read_char(TI_MSPBoot_MI_GetPhysicalAddressFromVirtual(addr)));
            __no_operation();
        }     
        // Double check current app

        // Erase download 
        for (addr = APP_START_ADDR; addr <= APP_END_ADDR; addr+=2){
            __data20_write_short(TI_MSPBoot_MI_GetPhysicalAddressFromVirtual(addr), 0xFFFF);
        }
        for (addr = APP2_START_ADDR; addr <= APP2_END_ADDR; addr+=2){
            __data20_write_short(TI_MSPBoot_MI_GetPhysicalAddressFromVirtual(addr), 0xFFFF);
        }
        // Software reset -- Applcation is validate, should enter in application from line 205    
        PMMCTL0 = PMMPW | PMMSWBOR;
    }
    else{
        
    }
}