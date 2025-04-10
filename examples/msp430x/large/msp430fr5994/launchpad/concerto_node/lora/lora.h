#define RH_BROADCAST_ADDRESS	0xff
#define RH_RF95_REG_01_OP_MODE	0x01
#define RH_RF95_MODE_SLEEP	0x00
#define RH_RF95_LONG_RANGE_MODE 0x80
#define RH_RF95_REG_0E_FIFO_TX_BASE_ADDR	0x0E
#define RH_RF95_REG_0F_FIFO_RX_BASE_ADDR	0x0F
#define RH_RF95_MODE_STDBY	0x01
#define RH_RF95_FXOSC	32000000.0
#define RH_RF95_FSTEP	(RH_RF95_FXOSC / 524288)
#define RH_RF95_REG_06_FRF_MSB	0x06
#define RH_RF95_REG_07_FRF_MID	0x07
#define RH_RF95_REG_08_FRF_LSB	0x08
#define RH_RF95_REG_20_PREAMBLE_MSB	0x20
#define RH_RF95_REG_21_PREAMBLE_LSB	0x21
#define RH_RF95_REG_4D_PA_DAC	0x4D
#define RH_RF95_PA_DAC_ENABLE	0x07
#define RH_RF95_PA_DAC_DISABLE	0x04
#define RH_RF95_REG_09_PA_CONFIG	0x09
#define RH_RF95_PA_SELECT	0x80
#define RH_RF95_REG_00_FIFO 0x00
#define RH_RF95_MODE_TX	0x03
#define RH_RF95_REG_40_DIO_MAPPING1	0x40
#define RH_RF95_REG_22_PAYLOAD_LENGTH 0x22
#define RH_RF95_HEADER_LEN	4
#define RH_SPI_WRITE_MASK	0x80
#define RH_RF95_REG_0D_FIFO_ADDR_PTR	0x0D
#define RH_RF95_MAX_PAYLOAD_LEN 255
#define RH_RF95_MAX_MESSAGE_LEN (RH_RF95_MAX_PAYLOAD_LEN - RH_RF95_HEADER_LEN)
#define RH_RF95_REG_1D_MODEM_CONFIG1	0x1D
#define RH_RF95_REG_1E_MODEM_CONFIG2	0x1E
#define RH_RF95_REG_26_MODEM_CONFIG3	0x26
#define RH_RF95_REG_13_RX_NB_BYTES      0x13
#define RH_RF95_REG_10_FIFO_RX_CURRENT_ADDR 0x10
#define RH_RF95_MODE_RXCONTINUOUS                     0x05

#define RH_RF95_TX_DONE                 0x08
#define RH_RF95_RX_DONE                 0x40
#define RH_RF95_VALID_HEADER            0x10
#define RH_RF95_REG_12_IRQ_FLAGS        0x12
#define RH_RF95_REG_12_IRQ_MASKS        0x11

void setModeIdle();
void setModeTx();
void setModeRx();
void setFrequency(float centre);
void setPreambleLength(uint16_t bytes);
void setTxPower(uint8_t power);
void waitPacketSent();
uint8_t send(const uint8_t* data, uint8_t len);
uint8_t recv(uint8_t* buf, uint8_t len);
void LoRa_init();