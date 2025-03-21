#include "tpl_os.h"
#include "tpl_os_event.h"

#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"

#include "em_adc.h"
#include "em_prs.h"
#include "em_timer.h"
#include "em_dma.h"
#include "em_gpio.h"
#include "em_ebi.h"
#include "em_burtc.h"
#include "em_opamp.h"

#include "dmactrl.h"
#include "diskio.h"

/* ----- Inference header ----- */
#include "matrix.h"
#include "model.h"

#include "neural_network_parameters_tpl.h"
#include "neural_network_struct.h"
#include "fixed_point_ops.h"
#include "layers.h"

#define ROUNDED_DIV(a, b)                         (((a) + ((b)/2)) / (b))
#define MIN(a, b)                                 ((a) < (b) ? (a) : (b))
#define MAX(a, b)                                 ((a) > (b) ? (a) : (b))

typedef struct {
	char id[4];
	uint32_t size;
  } chunk_t;

  typedef struct {
	chunk_t icmt;
	char comment[384];
  } icmt_t;
  
  typedef struct {
	chunk_t iart;
	char artist[32];
  } iart_t;

typedef struct {
	uint16_t format;
	uint16_t numberOfChannels;
	uint32_t samplesPerSecond;
	uint32_t bytesPerSecond;
	uint16_t bytesPerCapture;
	uint16_t bitsPerSample;
  } wavFormat_t;

typedef struct {
chunk_t riff;
char format[4];
chunk_t fmt;
wavFormat_t wavFormat;
chunk_t list;
char info[4];
icmt_t icmt;
iart_t iart;
chunk_t data;
} wavHeader_t;

/* First attempt SD card */
// typedef struct {
// 	char riff[4];
// 	int32_t flength;
// 	char wave[4];
// 	char fmt[4];
// 	int32_t chunk_size;
// 	int16_t format_tag;
// 	int16_t num_chan;
// 	int32_t sample_rate;
// 	int32_t byte_per_second;
// 	int16_t byte_per_sample;
// 	int16_t bits_per_sample;
// 	char data[4];
// 	int32_t dlength;
// } wavHeader_t;

FIL fileaudio;
static UINT bw;

#define APP_Task_blink_START_SEC_CODE
#include "tpl_memmap.h"
FUNC(int, OS_APPL_CODE) main(void){
	CHIP_Init();
	// Enable Clock to GPIO
	CMU_ClockEnable(cmuClock_GPIO, true);
	/* Enable high frequency HFXO clock */
    CMU_OscillatorEnable(cmuOsc_HFXO, true, true);
    CMU_ClockDivSet(cmuClock_HF, cmuClkDiv_1);
    CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
    CMU_OscillatorEnable(cmuOsc_HFRCO, false, false);
	/* Enable clock to Low energy module */
	CMU_ClockEnable(cmuClock_CORELE, true);
	/* Setup NVIC for ADC */ 
    // NVIC_ClearPendingIRQ(ADC0_IRQn);
    // NVIC_EnableIRQ(ADC0_IRQn);
	/* Setup NVIC for DMA */
	NVIC_ClearPendingIRQ(DMA_IRQn);
  	NVIC_EnableIRQ(DMA_IRQn);
	/* GPIO A */
	GPIO_PinModeSet(gpioPortA, 0, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortA, 1, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortA, 2, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortA, 3, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortA, 4, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortA, 5, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortA, 6, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortA, 7, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortA, 8, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortA, 9, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortA, 10, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortA, 11, gpioModePushPull, 0);
	GPIO_PinModeSet(gpioPortA, 12, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortA, 13, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortA, 14, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortA, 15, gpioModeDisabled, 0);
	/* GPIO B */
	GPIO_PinModeSet(gpioPortB, 0, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortB, 1, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortB, 2, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortB, 3, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortB, 4, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortB, 5, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortB, 6, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortB, 9, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortB, 10, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortB, 11, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortB, 12, gpioModeDisabled, 0);
	/* GPIO C */
	GPIO_PinModeSet(gpioPortC, 0, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortC, 1, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortC, 3, gpioModePushPull, 0);
	GPIO_PinModeSet(gpioPortC, 7, gpioModeDisabled, 0);	
	GPIO_PinModeSet(gpioPortC, 8, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortC, 9, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortC, 10, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortC, 11, gpioModeDisabled, 0);
	/* GPIO D */
    GPIO_PinModeSet(gpioPortD, 0, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortD, 1, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortD, 2, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortD, 3, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortD, 4, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortD, 5, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortD, 9, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortD, 10, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortD, 11, gpioModePushPull, 1);
	GPIO_PinModeSet(gpioPortD, 12, gpioModePushPull, 1);
	/* GPIO E */
	GPIO_PinModeSet(gpioPortE, 0, gpioModePushPull, 1);
	GPIO_PinModeSet(gpioPortE, 1, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortE, 2, gpioModePushPull, 1);
	GPIO_PinModeSet(gpioPortE, 3, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortE, 4, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortE, 5, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortE, 6, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortE, 7, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortE, 8, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortE, 9, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortE, 10, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortE, 11, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortE, 12, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortE, 13, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortE, 14, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortE, 15, gpioModeDisabled, 0);
	/* GPIO F */
	GPIO_PinModeSet(gpioPortF, 3, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortF, 4, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortF, 5, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortF, 6, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortF, 7, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortF, 8, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortF, 9, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortF, 12, gpioModeDisabled, 0);

	// Set GPIO Red Led to Output
	// Red Led on PC4
	GPIO->P[gpioPortC].MODEL |= 4 << (16);
	// Set GPIO Green Led to Output
	// Green Led on PC5
	GPIO->P[gpioPortC].MODEL |= 4 << (20);
	StartOS(OSDEFAULTAPPMODE);
	return 0;
}

TASK(blink){
	//Red Led
	GPIO->P[gpioPortC].DOUT ^= 1<<4;
	// Green Led
	GPIO->P[gpioPortC].DOUT ^= 1<<5;
	TerminateTask();
}
#define APP_Task_blink_STOP_SEC_CODE
#include "tpl_memmap.h"

#define APP_Task_start_audio_START_SEC_CODE
#include "tpl_memmap.h"


TASK(start_audio){

	GPIO_PinModeSet(gpioPortA, 14, gpioModeInput, 0);
    GPIO_IntConfig(gpioPortA, 14, true, true, true);
	bool microphone_ext = GPIO_PinInGet(gpioPortA, 14) == 0;
	// Data Microphone on PD7

	// Enable Microphone on PE0 --> Disable GPIO 
	GPIO_PinOutClear(gpioPortE, 0);
	// Enable VREF on PA11 to power Microphone
	GPIO_PinOutSet(gpioPortA, 11);
	/* Setup OpAmp */
	CMU_ClockEnable(cmuClock_DAC0, true);
	/* Define the configuration for OPA1 and OPA2 */
	OPAMP_Init_TypeDef opa1Init = OPA_INIT_INVERTING;
	OPAMP_Init_TypeDef opa2Init = OPA_INIT_INVERTING_OPA2;
	opa2Init.outPen = DAC_OPA2MUX_OUTPEN_OUT1;
	/* Set the gain */
    static OPAMP_ResSel_TypeDef opamp1NormalGainRange[] = {opaResSelR2eq4_33R1, opaResSelR2eq7R1, opaResSelR2eq15R1, opaResSelR2eq15R1, opaResSelR2eq15R1};
    static OPAMP_ResSel_TypeDef opamp2NormalGainRange[] = {opaResSelR2eqR1, opaResSelR2eqR1, opaResSelR2eqR1, opaResSelR1eq1_67R1, opaResSelR2eq2R1};
    static OPAMP_ResSel_TypeDef opamp1LowGainRange[] = {opaResSelR2eq0_33R1, opaResSelR2eq0_33R1, opaResSelR2eqR1, opaResSelR2eqR1, opaResSelR2eqR1};
    static OPAMP_ResSel_TypeDef opamp2LowGainRange[] = {opaResSelR2eqR1, opaResSelR1eq1_67R1, opaResSelR2eqR1, opaResSelR1eq1_67R1, opaResSelR2eq2R1};
    OPAMP_ResSel_TypeDef *opamp1Gain = 4 == 0 ? opamp1LowGainRange : opamp1NormalGainRange;
    OPAMP_ResSel_TypeDef *opamp2Gain = 4 == 0 ? opamp2LowGainRange : opamp2NormalGainRange;
    uint32_t index = MAX(0, MIN(1, 4));
    opa1Init.resSel = opamp1Gain[index];
    opa2Init.resSel = opamp2Gain[index];
	/* Enable OPA1 and OPA2 */
    OPAMP_Enable(DAC0, OPA1, &opa1Init);
    OPAMP_Enable(DAC0, OPA2, &opa2Init);
    /* Disable the clock */
    CMU_ClockEnable(cmuClock_DAC0, false);
	/* Now setup ADC */
	// Start the clock
	CMU_ClockEnable(cmuClock_ADC0, true);
	ADC_Reset(ADC0);
	// Setup ADC structure
	ADC_Init_TypeDef adcInit = ADC_INIT_DEFAULT;
	adcInit.prescale = (4 - 1);
	adcInit.warmUpMode = adcWarmupKeepADCWarm;
	adcInit.timebase = ADC_TimebaseCalc(0);
	adcInit.lpfMode = adcLPFilterRC;
	// Initialize ADC
	ADC_Init(ADC0, &adcInit);
	/* SCAN mode voltage reference must match the reference selected for SINGLE mode conversions */
	// Reset and set 2.5V Ref
	ADC0->SCANCTRL = ADC_SCANCTRL_REF_2V5;
	/* Configure ADC single conversion structure */
	ADC_InitSingle_TypeDef adcSingleInit = ADC_INITSINGLE_DEFAULT;
	adcSingleInit.prsSel = adcPRSSELCh0;
    adcSingleInit.reference = adcRef2V5;
	adcSingleInit.resolution = adcRes12Bit;
	adcSingleInit.input = adcSingleInpCh0Ch1;
	adcSingleInit.prsEnable = true;
    adcSingleInit.diff = true;
    adcSingleInit.rep = false;
	adcSingleInit.acqTime = adcAcqTime8;
	ADC_InitSingle(ADC0, &adcSingleInit);
	/* Enable ADC interrupt vector */
	ADC_IntClear(ADC0, ADC_IEN_SINGLE);
    // ADC_IntEnable(ADC0, ADC_IEN_SINGLE);
	/* NVIC is only possible on privileged mode, so only in kernel mode */
    // NVIC_ClearPendingIRQ(ADC0_IRQn);
    // NVIC_EnableIRQ(ADC0_IRQn);
	ChainTask(start_dma);
}
#define APP_Task_start_audio_STOP_SEC_CODE
#include "tpl_memmap.h"

#define APP_Task_start_dma_START_SEC_CODE
#include "tpl_memmap.h"
static int16_t primaryBuffer[1024];
static int16_t secondaryBuffer[1024];
const uint16_t numberOfSamplesPerTransfer = 1024;
static bool isPrimaryDMABuffer = true;

static void dma_callback(void){
	/* Switch buffer */
	// isPrimaryDMABuffer = !isPrimaryDMABuffer;
	// /* Re-activate the DMA */
    // DMA_RefreshPingPong(0,
    //     isPrimaryDMABuffer,						/* bool to change between dma descriptor, primaryBuffer or secondaryBuffer DST*/
    //     false,
    //     NULL,									/* dst : NULL = same as in descriptor, thus same as in start dma TASK, (void*)primaryBuffer or (void*)secondaryBuffer */
    //     NULL,									/* src : NULL = same as in descriptor, thus same as in start dma TASK, (void*)&(ADC0->SINGLEDATA) */
    //     numberOfSamplesPerTransfer - 1,
    //     false);
	/* With Trampoline, SetEvent to schedule the task dma buffer --> sram ext buffer */
	// SetEvent();
	return;
}



TASK(start_dma){
	/* ADC and Timer is started, now link to DMA */
	CMU_ClockEnable(cmuClock_DMA, true);
	/* Initialise the DMA structure */
    DMA_Init_TypeDef dmaInit;
	dmaInit.hprot = 0;
    dmaInit.controlBlock = dmaControlBlock;
	DMA_Init(&dmaInit);
	/* Configure call-back when DMA transfert done */
	DMA_CB_TypeDef cb;
	/*****************************************  CALLBACK DOESNT WORK !  ********************************************************/
	cb.cbFunc = dma_callback;
    cb.userPtr = NULL;
	/* Setup channel */
	DMA_CfgChannel_TypeDef chnlCfg;
	chnlCfg.highPri = false;
    chnlCfg.enableInt = true;
    chnlCfg.select = DMAREQ_ADC0_SINGLE;
	chnlCfg.cb = &cb;
    DMA_CfgChannel(0, &chnlCfg);
	/* Setting up channel descriptor */
    DMA_CfgDescr_TypeDef descrCfg;
    descrCfg.dstInc = dmaDataInc2;
    descrCfg.srcInc = dmaDataIncNone;
    descrCfg.size = dmaDataSize2;
    descrCfg.arbRate = dmaArbitrate1;
    descrCfg.hprot = 0;
	/* Set up both the primary and the secondary transfers */
    DMA_CfgDescr(0, true, &descrCfg);
    DMA_CfgDescr(0, false, &descrCfg);
	/* Set up the first transfer */
	isPrimaryDMABuffer = true;
	
    DMA_ActivatePingPong(0,
        false,
        (void*)primaryBuffer,
        (void*)&(ADC0->SINGLEDATA),
        numberOfSamplesPerTransfer - 1,
        (void*)secondaryBuffer,
        (void*)&(ADC0->SINGLEDATA),
        numberOfSamplesPerTransfer - 1);
	/* Enable SRAM EXTERN */
	// Start address is 0x80000000, size is 256 * 1024 bytes
	// Clear PD11 --> CPU_FET_SRAM_EN_N
	GPIO->P[gpioPortE].DOUTCLR = 1 << 11;
	/* Enable the external bus interface */
	/* Enable clocks */
    CMU_ClockEnable(cmuClock_EBI, true);
	/* Enable SRAM EBI D0..07 data pins (PortE 8 -- 15)*/
    GPIO_PinModeSet(gpioPortE, 8, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 9, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 10, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 11, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 12, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 13, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 14, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 15, gpioModePushPull, 0);
    /* Enable SRAM EBI A0..15 address pins (PortA 15, 0 -- 6, PortE 1, PortC 9 - 10, PortE 4 -- 7, PortC 8, PortB 0 - 1)*/
    GPIO_PinModeSet(gpioPortA, 15, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortA, 0, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortA, 1, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortA, 2, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortA, 3, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortA, 4, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortA, 5, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortA, 6, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 1, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortC, 9, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortC, 10, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 4, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 5, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 6, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 7, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortC, 8, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortB, 0, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortB, 1, gpioModePushPull, 0);
    /* Enable SRAM EBI CS0-CS1 (PortD 9 - 10)*/
    GPIO_PinModeSet(gpioPortD, 9, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortD, 10, gpioModePushPull, 0);
    /* Enable SRAM EBI WEN/OEN (PortF 8 - 9)*/
    GPIO_PinModeSet(gpioPortF, 8, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortF, 9, gpioModePushPull, 0);
	/* Configure EBI controller, changing default values */
	EBI_Init_TypeDef ebiInit = EBI_INIT_DEFAULT;
    ebiInit.mode = ebiModeD8A8;
    ebiInit.banks = EBI_BANK0;
    ebiInit.csLines = EBI_CS0 | EBI_CS1;
    ebiInit.readHalfRE = true;
    ebiInit.aLow = ebiALowA8;
    ebiInit.aHigh = ebiAHighA18;
	/* Address Setup and hold time */
    ebiInit.addrHoldCycles  = 0;
    ebiInit.addrSetupCycles = 0;
	/* Read cycle times */
    ebiInit.readStrobeCycles = 3;
    ebiInit.readHoldCycles   = 1;
    ebiInit.readSetupCycles  = 2;
	/* Write cycle times */
    ebiInit.writeStrobeCycles = 6;
    ebiInit.writeHoldCycles   = 0;
    ebiInit.writeSetupCycles  = 0;
	ebiInit.location = ebiLocation1;
    /* Configure EBI */
    EBI_Init(&ebiInit);


	/* Setup PRS (Peripheral Reflex System) between Timer and ADC */
	CMU_ClockEnable(cmuClock_PRS, true);
	CMU_ClockEnable(cmuClock_TIMER2, true);
	/* Connect PRS channel 0 to TIMER overflow */
	PRS_SourceSignalSet(0, PRS_CH_CTRL_SOURCESEL_TIMER2, PRS_CH_CTRL_SIGSEL_TIMER2OF, prsEdgeOff);
	/* Enable TIMER with default settings */
	TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
	timerInit.enable = false;
	TIMER_Init(TIMER2, &timerInit);
	/* Configure TIMER to trigger on sampling rate */
	TIMER_TopSet(TIMER2,  CMU_ClockFreqGet(cmuClock_TIMER2) / 8000 - 1);
	/* Enable Timer on ADC */
	TIMER_Enable(TIMER2, true);
	/* Start ADC sample */
	ADC_Start(ADC0, adcStartSingle);
	TerminateTask();
}
#define APP_Task_start_dma_STOP_SEC_CODE
#include "tpl_memmap.h"


#define APP_Task_copyDMAtoSRAM_START_SEC_CODE
#include "tpl_memmap.h"

#define SRAM_EXT_START_ADDR 0x80000000				/* EBI region 0 */

static int16_t buffer_sram[1024];

TASK(copyDMAtoSRAM){
	/* Configure DMA Channel 1 to transfer from DMA buffer to SRAM Extern */
	/* Initialise the DMA structure */
	// DMA_Init_TypeDef dmaInit1;
	// dmaInit1.hprot = 0;
	// dmaInit1.controlBlock = dmaControlBlock;
	// DMA_Init(&dmaInit1);
	// /* Setup channel */
	// DMA_CfgChannel_TypeDef chnlCfg1;
	// chnlCfg1.highPri = false;
    // chnlCfg1.enableInt = true;
	// chnlCfg1.select = 0; /* 0 because memory to memory transfer */
	// DMA_CfgChannel(1, &chnlCfg1);
	// /* Setting up channel descriptor */
    // DMA_CfgDescr_TypeDef descrCfg1;
	// descrCfg1.dstInc = dmaDataIncNone;
    // descrCfg1.srcInc = dmaDataIncNone;
    // descrCfg1.size = 1024;
	// descrCfg1.arbRate = dmaArbitrate1;
    // descrCfg1.hprot = 0;
	// /* Set up both the primary and the secondary transfers */
    // DMA_CfgDescr(1, true, &descrCfg1);
	/* Wait for event to transfer audio to sram */
	EventMaskType ev;
	WaitEvent(ev_DMAtoSRAM);
	GetEvent(copyDMAtoSRAM, ev);
	ClearEvent(ev);
	int16_t *buffer_dma;
	if(isPrimaryDMABuffer){
		buffer_dma = secondaryBuffer;
	}
	else{
		buffer_dma = primaryBuffer;
	}
	*buffer_sram = (int16_t *) SRAM_EXT_START_ADDR;
	for(uint16_t i = 0; i < 1024; i++){
		buffer_sram[i] = *buffer_dma++;
	}


	static int16_t check_buffer_sram[1024];
	for(uint16_t i = 0; i < 1024; i++){
		check_buffer_sram[i] = buffer_sram[i];
	}


	/* For testing only, we write 1 min to sd card, 8KHz --> 1024 points --> 469 buffers to write */
	static uint16_t cnt_write = 0;
	if(cnt_write < 1000){
		cnt_write++;
		f_open(&fileaudio, "audio.wav", FA_OPEN_APPEND | FA_WRITE);
		f_write(&fileaudio, check_buffer_sram, 2*1024, &bw);
		f_close(&fileaudio);
	}
	else{
		TerminateTask();
	}
	// DMA_ActivateBasic(1,			/* Channel 1*/
	// 	true,						/* primary cfg */
	// 	false,						/* no burst */
	// 	*buffer_sram,				/* dst */
	// 	*buffer_dma,				/* src */
	// 	1							/* 1 transfert */
	// );

	// static int16_t check_buffer_sram[1024];
	// for(uint16_t i = 0; i < 1024; i++){
	// 	check_buffer_sram[i] = buffer_sram[i];
	// }
	ChainTask(copyDMAtoSRAM);
	// TerminateTask();
}
#define APP_Task_copyDMAtoSRAM_STOP_SEC_CODE
#include "tpl_memmap.h"

#define APP_Task_start_sdcard_START_SEC_CODE
#include "tpl_memmap.h"

static FATFS fatfs;

static void setTime(uint32_t time, uint32_t milliseconds) {
	/* 1024 tick per seconds */
    uint32_t ticks = ROUNDED_DIV(1024 * milliseconds, 1000);
    uint64_t intendedCounter = 1024 * (uint64_t)time + ticks;
    uint64_t offset = intendedCounter - (uint64_t)BURTC_CounterGet();
    BURTC_RetRegSet(1, (uint32_t)(offset >> 32));
    BURTC_RetRegSet(0, (uint32_t)(offset & 0xFFFFFFFF));
    BURTC_RetRegSet(2, 0x11223344);
	return;
}

static void getTime(uint32_t *time, uint32_t *milliseconds) {

    uint64_t offset =  (uint64_t)BURTC_RetRegGet(1) << 32;
    offset += (uint64_t)BURTC_RetRegGet(0);
    uint64_t currentCounter = offset + BURTC_CounterGet();

    if (time != NULL) {
        *time = currentCounter / 1024;
    }

    if (milliseconds != NULL) {
        uint32_t ticks = currentCounter % 1024;
        *milliseconds = ROUNDED_DIV(1000 * ticks, 1024);
    }
	return;
}

static void handleTimeOverflow(void) {
    uint32_t offsetHigh = BURTC_RetRegGet(1);
    BURTC_RetRegSet(1, offsetHigh + 1);
}

/* Time function for FAT file system */
DWORD get_fattime(void) {

    int8_t timezoneHours = 0;

    int8_t timezoneMinutes = 0;

    uint32_t currentTime;

    getTime(&currentTime, NULL);

    if (BURTC_IntGet() & BURTC_IF_OF) {
        handleTimeOverflow();
        getTime(&currentTime, NULL);
        BURTC_IntClear(BURTC_IF_OF);
    }

    time_t fatTime = currentTime + timezoneHours * 60 * 60 + timezoneMinutes * 60;

    struct tm timePtr;
	// return 0;
    gmtime_r(&fatTime, &timePtr);

    return (((unsigned int)timePtr.tm_year - 208) << 25) |
            (((unsigned int)timePtr.tm_mon + 1 ) << 21) |
            ((unsigned int)timePtr.tm_mday << 16) |
            ((unsigned int)timePtr.tm_hour << 11) |
            ((unsigned int)timePtr.tm_min << 5) |
            ((unsigned int)timePtr.tm_sec >> 1);
}


TASK(start_sdcard){
	GPIO_PinModeSet(gpioPortD, 12, gpioModePushPull, 1);
	/* Turn on SD card (PortD 12)*/
	GPIO_PinOutClear(gpioPortD, 12);
	/* Init */
	MICROSD_Init();
	/* Check SD card status */
	DSTATUS resCard = disk_initialize(0);
	if (resCard == STA_NOINIT || resCard == STA_NODISK || resCard == STA_PROTECT) {
        while(1);
    }
    /* Initialise file system */
    if (f_mount(&fatfs, "", 1) != FR_OK) {
        while(1);
    }

	const int duration_in_second = 128;				/* For example 1024/8000 = 0.128 s */


  	f_open(&fileaudio, "audio.wav", FA_OPEN_APPEND | FA_WRITE);
	/* Header detail of wav file */
	static wavHeader_t wavHeader = {
	 .riff = {.id = "RIFF", .size = 0},
		.format = "WAVE",
		.fmt = {.id = "fmt ", .size = sizeof(wavFormat_t)},
		.wavFormat = {.format = 1,							/* PCM format */
					  .numberOfChannels = 1,
					  .samplesPerSecond = 8000,				/* 8 KHz */
					  .bytesPerSecond = 2*8000,
					  .bytesPerCapture = 2,
					  .bitsPerSample = 16},
		.list = {.id = "LIST",
				 .size = 4 + sizeof(icmt_t) + sizeof(iart_t)},
		.info = "INFO",
		.icmt = {.icmt.id = "ICMT", .icmt.size = 0, .comment = ""},
		.iart = {.iart.id = "IART", .iart.size = 0, .artist = ""},
		.data = {.id = "data", .size = 0}
	};

	wavHeader.wavFormat.samplesPerSecond = 8000;
	wavHeader.wavFormat.bytesPerSecond = 2 * 8000;
	wavHeader.data.size = 2 * 8000 * duration_in_second;
	wavHeader.riff.size = 2 * 8000 * duration_in_second +
						   sizeof(wavHeader_t) - sizeof(chunk_t);
	f_write(&fileaudio, &wavHeader, sizeof(wavHeader_t), &bw);
	f_close(&fileaudio);
	TerminateTask();

}
#define APP_Task_start_sdcard_STOP_SEC_CODE
#include "tpl_memmap.h"

// #define APP_ISR_isr_adc_START_SEC_CODE
// #include "tpl_memmap.h"
// ISR(isr_adc){
// 	// /* Get the interrupt mask */
// 	// uint32_t interruptMask = ADC_IntGet(ADC0);
// 	// /* Clear the interrupt */
//     // ADC_IntClear(ADC0, interruptMask);
// 	// if (interruptMask & ADC_IF_SINGLE) {
//     // 	/* Send the sample to the interrupt handler */
//     //     int16_t sample = ADC_DataSingleGet(ADC0);
// 	// }
// }
// #define APP_ISR_isr_adc_STOP_SEC_CODE
// #include "tpl_memmap.h"

#define APP_ISR_isr_dma_START_SEC_CODE
#include "tpl_memmap.h"
ISR(isr_dma){
	/* Get interrupt mask */
	uint32_t interruptMask = DMA_IntGet();
	if(interruptMask == DMA_IF_CH0DONE){
		/* Clear interrupt */
		DMA_IntClear(DMA_IFC_CH0DONE);
		isPrimaryDMABuffer = !isPrimaryDMABuffer;
		/* Re-activate the DMA */
    	DMA_RefreshPingPong(0,
        	isPrimaryDMABuffer,						/* bool to change between dma descriptor, primaryBuffer or secondaryBuffer DST*/
        	false,
        	NULL,									/* dst : NULL = same as in descriptor, thus same as in start dma TASK, (void*)primaryBuffer or (void*)secondaryBuffer */
        	NULL,									/* src : NULL = same as in descriptor, thus same as in start dma TASK, (void*)&(ADC0->SINGLEDATA) */
        	numberOfSamplesPerTransfer - 1,
        	false);
		SetEvent(copyDMAtoSRAM, ev_DMAtoSRAM);
	}
	else if (interruptMask == DMA_IF_CH1DONE){
		/* nothing to do on DMA ch 1 ? */
	}
}
#define APP_ISR_isr_dma_STOP_SEC_CODE
#include "tpl_memmap.h"

#define APP_Task_preprocess_START_SEC_CODE
#include "tpl_memmap.h"

typedef struct {
	float xv[11];
	float yv[11];
} BW_filter_t;

VAR(float, AUTOMATIC) a_band1 [11] = {
	0.15321141,
	-0.5377237,
	1.8085212,
	-3.52529355,
	6.34467972,
	-7.95807998,
	9.18825376,
	-7.42000549,
	5.52502756,
	-2.42000723,
	1.
};

VAR(float, AUTOMATIC) b [11] = {
	-0.00084414,
	0.,
	0.00422071,
	0.,
	-0.00844141,
	0.,
	0.00844141,
	0.,
	-0.00422071,
	0.,
	0.00084414
};

float Butterworth_applyBandPassFilter(float *a, float *b, float sample, BW_filter_t *filter){
	/* Shift x filter buffer */
	uint8_t i;
	for(i = 0; i < 11-1; i++){
		filter->xv[i] = filter->xv[i+1];
	}
	filter->xv[10] = sample;

	/* sum(x[n-k] * b[k]) */
	float feed_fwd = 0;
	for(i = 0; i < 11; i++){
		feed_fwd += b[i] * filter->xv[i];
	}

	/* Shift y filter buffer */
	for(i = 0; i < 11-1; i++){
		filter->yv[i] = filter->yv[i+1];
	}

	/* sum(y[n-k] * a[k]) */
	float feed_bck = 0;
	for(i = 0; i < 11; i++){
		feed_bck += a[i] * filter->yv[i];
	}

	filter->yv[10] = feed_fwd - feed_bck;

	/* Need to return value ? */
	return filter->yv[10];

}

TASK(preprocess){
	const int frame_length = 1048;
	const int hop_length = 128;

	const sample_length = 10;

	BW_filter_t filter_buffer = {{0}, {0}};
	uint16_t count = 0; 
	uint16_t count_frame = 0;
	
	for(uint16_t i = 0; i < sample_length; i++){
		float y_filter = Butterworth_applyBandPassFilter(a_band1, b, i, &filter_buffer);
	}
	TerminateTask();
}

#define APP_Task_preprocess_STOP_SEC_CODE
#include "tpl_memmap.h"

#define APP_Task_inference_START_SEC_CODE
#include "tpl_memmap.h"
/* Output of a layer should not exceed 2720 bytes, ie AA0*/
VAR(dtype, AUTOMATIC) MODEL_ARRAY_OUTPUT[0xAA0] = {0};
VAR(dtype, AUTOMATIC) MODEL_ARRAY_TEMP[0xAA0] = {0};
VAR(dtype, AUTOMATIC) input_buffer[1] = {0};
VAR(dtype, AUTOMATIC) output_buffer[1] = {0};
VAR(dtype, AUTOMATIC) label;

void input_output(matrix *output, matrix *input, uint16_t numFilters){
    /* copy output matrix and reference input to copied output */
    dma_load(MODEL_ARRAY_TEMP, output->data, output->numRows * output->numCols * numFilters);
    input->data = MODEL_ARRAY_TEMP;
    input->numRows = output->numRows;
    input->numCols = output->numCols;
}

int dma_load(int16_t* dest, int16_t* src, int n){
    int16_t i;
    for(i=n-1; i>=0; i--){
        dest[i] = src[i];
    }
    return n;
}


VAR(matrix, AUTOMATIC) inputFeatures = {
    .numRows = 1,
    .numCols = 1,
    .data = input_buffer,
};

VAR(matrix, AUTOMATIC) outputLabels = {
    .numRows = 1,
    .numCols = 1,
    .data = output_buffer,
};


TASK(inference){
	inputFeatures.data = input_buffer;
	inputFeatures.numRows = 1;
	inputFeatures.numCols = 1;

	outputLabels.data = output_buffer;
	outputLabels.numRows = 1;
	outputLabels.numCols = 1;

	matrix *input = &inputFeatures;
	matrix *output = &outputLabels;

	int16_t *bias_array;

	matrix kernel;
	matrix bias;
	uint16_t layer_class, activation, numChannels, filter_numRows, filter_numCols, stride_numRows, stride_numCols, filters_length, padding;
    uint16_t numFilters;

	/* ----- Conv1D_2 ----- */
	activation = conv1d_2_layer.activation;
	numFilters = conv1d_2_layer.numFilter;
	numChannels = conv1d_2_layer.numChannels;
	filter_numRows = conv1d_2_layer.filterRows;
	filter_numCols = conv1d_2_layer.filterCols;
	stride_numRows = conv1d_2_layer.strideRows;
	stride_numCols = conv1d_2_layer.strideCols;
	filters_length = conv1d_2_layer.filterLen;
	padding = conv1d_2_layer.padding;

	if(padding==1){
		output->numRows = input->numRows / stride_numRows;
		if(input->numRows % stride_numRows > 0){
			output->numRows++;
		}
		output->numCols = input->numCols / stride_numCols;
		if(input->numCols % stride_numCols > 0){
			output->numCols++;
		}
	}
	else{
		output->numRows = (input->numRows - filter_numRows + 1) / stride_numRows;
        if ((input->numRows - filter_numRows + 1) % stride_numRows > 0){
            output->numRows ++;
        }
        output->numCols = (input->numCols - filter_numCols + 1) / stride_numCols;
        if ((input->numCols - filter_numCols + 1) % stride_numCols > 0){
            output->numCols ++;
        }
	}
	int16_t *filters_array = conv1d_2_layer.weight;
    matrix filters = {filters_array, filter_numRows, filter_numCols};
    bias_array = conv1d_2_layer.bias;
	// execute conv1d layer
    if (activation == RELU_ACTIVATION){
        conv2d(output, input, &filters, numFilters, numChannels, bias_array, &fp_relu, FIXED_POINT_PRECISION, stride_numRows, stride_numCols, padding);
    }
    else if (activation == SIGMOID_ACTIVATION){
        conv2d(output, input, &filters, numFilters, numChannels, bias_array, &fp_sigmoid, FIXED_POINT_PRECISION, stride_numRows, stride_numCols, padding);
    }
    else{
        conv2d(output, input, &filters, numFilters, numChannels, bias_array, &fp_linear, FIXED_POINT_PRECISION, stride_numRows, stride_numCols, padding);
    }
	input_output(output, input, numFilters);

	/* ----- MAX POOLING_2 ----- */
	uint16_t pool_numRows = max_pooling1d_2_layer.poolRows;
    uint16_t pool_numCols = max_pooling1d_2_layer.poolCols;
    stride_numRows = max_pooling1d_2_layer.strideRows;
    stride_numCols = max_pooling1d_2_layer.strideCols;
    padding = max_pooling1d_2_layer.padding;

    output->numRows = input->numRows / pool_numRows;
    output->numCols = input->numCols / pool_numCols;
	maxpooling_filters(output, input, numFilters, pool_numRows, pool_numCols);
    input_output(output, input, numFilters);

	/* ----- Conv1D_3 ----- */
	activation = conv1d_3_layer.activation;
	numFilters = conv1d_3_layer.numFilter;
	numChannels = conv1d_3_layer.numChannels;
	filter_numRows = conv1d_3_layer.filterRows;
	filter_numCols = conv1d_3_layer.filterCols;
	stride_numRows = conv1d_3_layer.strideRows;
	stride_numCols = conv1d_3_layer.strideCols;
	filters_length = conv1d_3_layer.filterLen;
	padding = conv1d_3_layer.padding;

	if(padding==1){
		output->numRows = input->numRows / stride_numRows;
		if(input->numRows % stride_numRows > 0){
			output->numRows++;
		}
		output->numCols = input->numCols / stride_numCols;
		if(input->numCols % stride_numCols > 0){
			output->numCols++;
		}
	}
	else{
		output->numRows = (input->numRows - filter_numRows + 1) / stride_numRows;
        if ((input->numRows - filter_numRows + 1) % stride_numRows > 0){
            output->numRows ++;
        }
        output->numCols = (input->numCols - filter_numCols + 1) / stride_numCols;
        if ((input->numCols - filter_numCols + 1) % stride_numCols > 0){
            output->numCols ++;
        }
	}
	filters_array = conv1d_3_layer.weight;
	filters.data = filters_array;
    filters.numRows = filter_numRows;
    filters.numCols = filter_numCols;    
	bias_array = conv1d_3_layer.bias;
	// execute conv1d layer
    if (activation == RELU_ACTIVATION){
        conv2d(output, input, &filters, numFilters, numChannels, bias_array, &fp_relu, FIXED_POINT_PRECISION, stride_numRows, stride_numCols, padding);
    }
    else if (activation == SIGMOID_ACTIVATION){
        conv2d(output, input, &filters, numFilters, numChannels, bias_array, &fp_sigmoid, FIXED_POINT_PRECISION, stride_numRows, stride_numCols, padding);
    }
    else{
        conv2d(output, input, &filters, numFilters, numChannels, bias_array, &fp_linear, FIXED_POINT_PRECISION, stride_numRows, stride_numCols, padding);
    }
	input_output(output, input, numFilters);

	/* ----- MAX POOLING_3 ----- */
	pool_numRows = max_pooling1d_3_layer.poolRows;
    pool_numCols = max_pooling1d_3_layer.poolCols;
    stride_numRows = max_pooling1d_3_layer.strideRows;
    stride_numCols = max_pooling1d_3_layer.strideCols;
    padding = max_pooling1d_3_layer.padding;

    output->numRows = input->numRows / pool_numRows;
    output->numCols = input->numCols / pool_numCols;
	maxpooling_filters(output, input, numFilters, pool_numRows, pool_numCols);
    input_output(output, input, numFilters);

	/* ----- Conv1D_4 ----- */
	activation = conv1d_4_layer.activation;
	numFilters = conv1d_4_layer.numFilter;
	numChannels = conv1d_4_layer.numChannels;
	filter_numRows = conv1d_4_layer.filterRows;
	filter_numCols = conv1d_4_layer.filterCols;
	stride_numRows = conv1d_4_layer.strideRows;
	stride_numCols = conv1d_4_layer.strideCols;
	filters_length = conv1d_4_layer.filterLen;
	padding = conv1d_4_layer.padding;

	if(padding==1){
		output->numRows = input->numRows / stride_numRows;
		if(input->numRows % stride_numRows > 0){
			output->numRows++;
		}
		output->numCols = input->numCols / stride_numCols;
		if(input->numCols % stride_numCols > 0){
			output->numCols++;
		}
	}
	else{
		output->numRows = (input->numRows - filter_numRows + 1) / stride_numRows;
        if ((input->numRows - filter_numRows + 1) % stride_numRows > 0){
            output->numRows ++;
        }
        output->numCols = (input->numCols - filter_numCols + 1) / stride_numCols;
        if ((input->numCols - filter_numCols + 1) % stride_numCols > 0){
            output->numCols ++;
        }
	}
	filters_array = conv1d_4_layer.weight;
	filters.data = filters_array;
    filters.numRows = filter_numRows;
    filters.numCols = filter_numCols;    
	bias_array = conv1d_4_layer.bias;
	// execute conv1d layer
    if (activation == RELU_ACTIVATION){
        conv2d(output, input, &filters, numFilters, numChannels, bias_array, &fp_relu, FIXED_POINT_PRECISION, stride_numRows, stride_numCols, padding);
    }
    else if (activation == SIGMOID_ACTIVATION){
        conv2d(output, input, &filters, numFilters, numChannels, bias_array, &fp_sigmoid, FIXED_POINT_PRECISION, stride_numRows, stride_numCols, padding);
    }
    else{
        conv2d(output, input, &filters, numFilters, numChannels, bias_array, &fp_linear, FIXED_POINT_PRECISION, stride_numRows, stride_numCols, padding);
    }
	input_output(output, input, numFilters);

	/* ----- Global Average Pooling 1D -----*/
	output->numRows = numFilters;
	output->numCols = 1;
	globalAveragePooling(output, input, numFilters);
    numFilters = 1;
    input_output(output, input, numFilters);
	/* ----- DENSE_4 -----*/
    numFilters = 1;
    activation = dense_4_layer.activation;

    output->numRows = dense_4_layer.kernel_numRows;
    output->numCols = input->numCols;

    kernel.data = dense_4_layer.weight;
    kernel.numRows = dense_4_layer.kernel_numRows;
    kernel.numCols = dense_4_layer.kernel_numCols;

    bias.data = dense_4_layer.bias;
    bias.numRows = dense_4_layer.bias_numRows;
    bias.numCols = dense_4_layer.bias_numCols;

    if (activation == RELU_ACTIVATION){
        dense(output, input, &kernel, &bias, &fp_relu, FIXED_POINT_PRECISION);
    }
    else if (activation == SIGMOID_ACTIVATION){
        dense(output, input, &kernel, &bias, &fp_sigmoid, FIXED_POINT_PRECISION);
    }
    else{
        dense(output, input, &kernel, &bias, &fp_linear, FIXED_POINT_PRECISION);
    }
    input_output(output, input, numFilters);
	/* ----- DENSE_5 -----*/
    numFilters = 1;
    activation = dense_5_layer.activation;

    output->numRows = dense_5_layer.kernel_numRows;
    output->numCols = input->numCols;

    kernel.data = dense_5_layer.weight;
    kernel.numRows = dense_5_layer.kernel_numRows;
    kernel.numCols = dense_5_layer.kernel_numCols;

    bias.data = dense_5_layer.bias;
    bias.numRows = dense_5_layer.bias_numRows;
    bias.numCols = dense_5_layer.bias_numCols;

    if (activation == RELU_ACTIVATION){
        dense(output, input, &kernel, &bias, &fp_relu, FIXED_POINT_PRECISION);
    }
    else if (activation == SIGMOID_ACTIVATION){
        dense(output, input, &kernel, &bias, &fp_sigmoid, FIXED_POINT_PRECISION);
    }
    else{
        dense(output, input, &kernel, &bias, &fp_linear, FIXED_POINT_PRECISION);
    }
    input_output(output, input, numFilters);

	/* Classification */
	label = argmax(output);
	TerminateTask();
}
#define APP_Task_inference_STOP_SEC_CODE
#include "tpl_memmap.h"