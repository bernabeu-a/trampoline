#include "msp430.h"
#include <stdint.h>

#include "lora.h"

/* Internal Function for SPI */

uint8_t singleTransfer(uint8_t address, uint8_t value)
{
    uint8_t response;

    P3OUT &= ~BIT0;                         // CS (NSS) LOW - Chip Select Lora Chip
    while (!(UCA3IFG & UCTXIFG));           // USCI_B0 TX buffer ready?
        UCA3TXBUF = address;                // Send address over SPI to Slave
    while (!(UCA3IFG & UCRXIFG));           // USCI_B0 RX Received?
        response = UCA3RXBUF;               // Store received data

    while (!(UCA3IFG & UCTXIFG));           // USCI_B0 TX buffer ready?
        UCA3TXBUF = value;                  // Send value over SPI to Slave
    while (!(UCA3IFG & UCRXIFG));           // USCI_B0 RX Received?
        response = UCA3RXBUF;
    P3OUT |= BIT0;                          // CS (NSS) HIGH - Release Lora Chip

    return response;
}

void writeBurstRegister(uint8_t reg, const uint8_t* src, uint8_t len){
	P3OUT &= ~BIT0;
	uint8_t response;
	while (!(UCA3IFG & UCTXIFG));           // USCI_B0 TX buffer ready?
        UCA3TXBUF = reg | RH_SPI_WRITE_MASK;                // Send address over SPI to Slave
    while (!(UCA3IFG & UCRXIFG));           // USCI_B0 RX Received?
        response = UCA3RXBUF;               // Store received data
	while(len--) {
		while (!(UCA3IFG & UCTXIFG));           // USCI_B0 TX buffer ready?
		UCA3TXBUF = *src++;                  // Send value over SPI to Slave
		while (!(UCA3IFG & UCRXIFG));           // USCI_B0 RX Received?
        response = UCA3RXBUF;
	}
	P3OUT |= BIT0;
	return;
}

void readBurstRegister(uint8_t reg, uint8_t* dest, uint8_t len){
    uint8_t response;

  	P3OUT &= ~BIT0;
	while (!(UCA3IFG & UCTXIFG));                   // USCI_B0 TX buffer ready?
        UCA3TXBUF = reg & ~RH_SPI_WRITE_MASK;       // Send address over SPI to Slave
    while (!(UCA3IFG & UCRXIFG));                   // USCI_B0 RX Received?
        response = UCA3RXBUF;                       // Store received data
	while(len--) {
		while (!(UCA3IFG & UCTXIFG));           // USCI_B0 TX buffer ready?
		UCA3TXBUF = 0x00;                       // Send value over SPI to Slave
		while (!(UCA3IFG & UCRXIFG));           // USCI_B0 RX Received?
		response = UCA3RXBUF;
        *dest++ = response;

	}
	P3OUT |= BIT0;
	return;
}
uint8_t readRegister(uint8_t address)
{
    return singleTransfer(address & 0x7f,0x00);
}

void writeRegister(uint8_t address, uint8_t value)
{
    singleTransfer(address | 0x80,value);
}

/* Function for LoRa */

void setModeIdle(){
	writeRegister(RH_RF95_REG_01_OP_MODE, RH_RF95_MODE_STDBY);
	return;
}

void setModeTx(){
	writeRegister(RH_RF95_REG_01_OP_MODE, RH_RF95_MODE_TX);
	writeRegister(RH_RF95_REG_40_DIO_MAPPING1, 0x40);
	return;
}

void setModeRx(){
	writeRegister(RH_RF95_REG_01_OP_MODE, RH_RF95_MODE_RXCONTINUOUS);
	writeRegister(RH_RF95_REG_40_DIO_MAPPING1, 0x40);
	return;
}

void setFrequency(float centre){
	uint32_t frf = (centre * 1000000.0)/RH_RF95_FSTEP;
	writeRegister(RH_RF95_REG_06_FRF_MSB, (frf >>16) & 0xff);
	writeRegister(RH_RF95_REG_07_FRF_MID, (frf >>8) & 0xff);
	writeRegister(RH_RF95_REG_08_FRF_LSB, frf & 0xff);
	return;
}

void setPreambleLength(uint16_t bytes){
	writeRegister(RH_RF95_REG_20_PREAMBLE_MSB, bytes >> 8);
	writeRegister(RH_RF95_REG_21_PREAMBLE_LSB, bytes & 0xff);
	return;
}

void setTxPower(uint8_t power){
	if(power > 23) power = 23;
	if(power < 5) power = 5;
	
	if(power > 20){
		writeRegister(RH_RF95_REG_4D_PA_DAC, RH_RF95_PA_DAC_ENABLE);
		power -= 3;
	}
	else{
		writeRegister(RH_RF95_REG_4D_PA_DAC, RH_RF95_PA_DAC_DISABLE);
	}
	writeRegister(RH_RF95_REG_09_PA_CONFIG, RH_RF95_PA_SELECT | (power-5));
	return;
}

void waitPacketSent(){
	while((readRegister(RH_RF95_REG_12_IRQ_FLAGS) != RH_RF95_TX_DONE));
    writeRegister(RH_RF95_REG_12_IRQ_FLAGS, RH_RF95_TX_DONE);
	setModeIdle();
    // set:ModeRx();
}

uint8_t send(const uint8_t* data, uint8_t len){
	if(len>RH_RF95_MAX_MESSAGE_LEN) return 0;
	while(readRegister(RH_RF95_REG_01_OP_MODE) == (RH_RF95_MODE_TX + RH_RF95_LONG_RANGE_MODE));
	//waitPacketSent();
	setModeIdle();
	
	writeRegister(RH_RF95_REG_0D_FIFO_ADDR_PTR, 0);
	uint8_t txHeaderTo = RH_BROADCAST_ADDRESS;
	uint8_t txHeaderFrom = RH_BROADCAST_ADDRESS;
	uint8_t txHeaderId = 0;
	uint8_t txHeaderFlags = 0;
	
	writeRegister(RH_RF95_REG_00_FIFO, txHeaderTo);
	writeRegister(RH_RF95_REG_00_FIFO, txHeaderFrom);
	writeRegister(RH_RF95_REG_00_FIFO, txHeaderId);
	writeRegister(RH_RF95_REG_00_FIFO, txHeaderFlags);
	
	writeBurstRegister(RH_RF95_REG_00_FIFO, data, len);
	writeRegister(RH_RF95_REG_22_PAYLOAD_LENGTH, len + RH_RF95_HEADER_LEN);
	
	setModeTx();

    waitPacketSent();
	return 1;
}

uint8_t recv(uint8_t* buf, uint8_t len){
    setModeRx();
 	while(readRegister(RH_RF95_REG_01_OP_MODE) != (RH_RF95_MODE_RXCONTINUOUS + RH_RF95_LONG_RANGE_MODE));
    uint8_t tmp_buf[228] = {0};
    // We wait for the packet to arrive
    while((readRegister(RH_RF95_REG_12_IRQ_FLAGS) != (RH_RF95_RX_DONE | RH_RF95_VALID_HEADER)));
	// Have received a packet
	uint8_t len_pkt = readRegister(RH_RF95_REG_13_RX_NB_BYTES);
	// Reset the fifo read ptr to the beginning of the packet
	writeRegister(RH_RF95_REG_0D_FIFO_ADDR_PTR, readRegister(RH_RF95_REG_10_FIFO_RX_CURRENT_ADDR));
	readBurstRegister(RH_RF95_REG_00_FIFO, tmp_buf, len_pkt);
    // Clear Rx flag
	writeRegister(RH_RF95_REG_12_IRQ_FLAGS, 0xFF);


    // Check whether the latest received message is complete and uncorrupted
    // uint8_t mesgTo = buf[0];
    // uint8_t mesgFrom = buf[1];
    // uint8_t mesgId = buf[2];
    // uint8_t mesgFlag = buf[3];

    if (buf && len){
        // Skip the 4 headers that are at the beginning of the tmp_buf
	    if (len > len_pkt-RH_RF95_HEADER_LEN) len = len_pkt-RH_RF95_HEADER_LEN;

        char *d = buf;
        const char *s = tmp_buf+RH_RF95_HEADER_LEN;
        while (len--) *d++ = *s++;
    }
	setModeIdle();
}

void LoRa_init(){
    //--- SPI pinOut Configuration ---//
    P3OUT  |= BIT0;                             // Set P3.0 HIGH (LoRa chip is selected)
    P3DIR  |= BIT0;                             // Set P3.0 as OUTPUT for CS
    P6SEL0 |= BIT0 + BIT1 + BIT2;               // P6.0 = MOSI, P6.1 = MISO, P6.2 = CLK,
    P6SEL1 &= ~(BIT0 + BIT1 + BIT2);            // P6.0 = MOSI, P6.1 = MISO, P6.2 = CLK

    //--- USCI module configuration for SPI transfert ---//
    UCA3CTLW0 = UCSWRST;                        // USCI UCB1 - configuration mode
    UCA3CTLW0 |= UCMSB + UCMST + UCSYNC;         // SET SPI : 3-pin, 8-bit SPI master      
    UCA3CTL1 |= UCSSEL_2;                       // Clock source Select
    UCA3BR0 = 2;                                // Clock divider by 1
    UCA3BR1 = 0;                                //
    UCA3CTLW0 &= ~UCSWRST;                      // USCI - SPI Activate

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    /* LoRa */
    writeRegister(RH_RF95_REG_01_OP_MODE | RH_SPI_WRITE_MASK, RH_RF95_MODE_SLEEP | RH_RF95_LONG_RANGE_MODE);
    
    long wait = 32600;
    long w;
    for(w=0; w<wait; w++){
    }
    if(readRegister(RH_RF95_REG_01_OP_MODE) != (RH_RF95_MODE_SLEEP | RH_RF95_LONG_RANGE_MODE)){
    }
    writeRegister(RH_RF95_REG_0E_FIFO_TX_BASE_ADDR, 0);
    writeRegister(RH_RF95_REG_0F_FIFO_RX_BASE_ADDR, 0);

    setModeIdle();

    setPreambleLength(8);
    setFrequency(868.0);
    setTxPower(5);
}
