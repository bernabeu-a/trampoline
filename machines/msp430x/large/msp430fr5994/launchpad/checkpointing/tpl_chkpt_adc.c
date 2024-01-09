#if WITH_CHECKPOINTING == YES
#include "msp430.h"
#include "adc12_b.h"
#include "tpl_os.h"

#define APP_COMMON_START_SEC_CODE
#include "tpl_memmap.h"

//global variable
VAR(uint16_t,AUTOMATIC) adc_conv_ready = 0;
// VAR(uint16_t,AUTOMATIC) result_adc = 0;

// #define USE_DMA_ADC 1
/*----------------------------------------------------------------------------*/
/* tpl_adc_init                                                               */
/*----------------------------------------------------------------------------*/
FUNC(void, OS_APPL_CODE) tpl_adc_init_simple(bool use1V2Ref, uint16_t *ptr_result_adc)
{
  // static bool use1V2Ref = true;
  // #if USE_DMA_ADC
  // uint16_t result_adc;
	DMA0CTL &= ~DMAEN__ENABLE;

  ADC12CTL0 &= ~ADC12ENC;                     // disable ADC
  ADC12CTL0 = ADC12ON | ADC12SHT0_4;          // turn ADC ON; sample + hold @ 64 × ADC12CLKs
  ADC12CTL1 = ADC12SSEL_0 | ADC12DIV_7;       // ADC12OSC as ADC12CLK (~5MHz) / 8
  ADC12CTL3 = ADC12TCMAP | ADC12BATMAP;       // Map Temp and BAT
  ADC12CTL1 |= ADC12SHP;                      // ADCCLK = MODOSC; sampling timer
  ADC12CTL2 |= ADC12RES_2;                    // 12-bit resolution
  // ADC12CTL2 |= ADC12DF;
  ADC12CTL2 &= ~ADC12DF;
  ADC12IFGR0 = 0;                             // Clear Flags

  // 	// For safety, protect RMW Cpu instructions
	// DMACTL4 |= DMARMWDIS;

	// DMACTL1 &= ~DMA2TSEL;			//reinit trigger src for DMA2
	// DMACTL1 |= DMA2TSEL__ADC12IFG;	// and set trigger Source (26 = ADC end of conversion)

  // // Initialize the DMA. Using DMA channel 2.
  // DMA2CTL = DMADT_0      |	//single transfer
	// 	    DMADSTBYTE__WORD |	//src is word
	// 		  DMADSTBYTE__WORD |	//dst is word
	// 		  DMALEVEL__LEVEL  |	//trigger on rising edge
	// 		  DMASRCINCR_0     |	//src is unchanged
	// 		  DMADSTINCR_0     ;	//dst is incremented
  // DMA2SZ = 1;			// Define DMA transfer size

  // DMA2SA = &ADC12MEM0;			// Set src address
  // DMA2DA = ptr_result_adc;				// Set dst address
  // DMA2CTL &= ~DMAIFG;
  // DMA2CTL |= DMAIE;				// Enable DMA interrupt 
  // #endif /* USE_DMA_ADC */

  // ADC12IER0 |= ADC12IE0;                      // Enable interrupts
  while (REFCTL0 & REFGENBUSY);               // If ref generator busy, WAIT
  if (use1V2Ref) {
    REFCTL0 = REFON | REFVSEL_0;                // 1.2V reference
    ADC12MCTL0 = ADC12INCH_31 | ADC12VRSEL_1;   // idem. Channel 31 is AVCC/2
  }
  else {
    REFCTL0 = REFON | REFVSEL_1;                // 2V reference
    ADC12MCTL0 = ADC12INCH_31 | ADC12VRSEL_1;   // idem. Channel 31 is AVCC/2
  }

  if (REFCTL0 & REFON) while (!(REFCTL0 & REFGENRDY)); // wait till ref generator ready

  // ADC12CTL0 |= ADC12ENC;
}

FUNC(void, OS_APPL_CODE) tpl_adc_init(void)
{
	//Initialize the ADC12B Module
  /*
   * Base address of ADC12B Module
   * Use internal ADC12B bit as sample/hold signal to start conversion
   * USE MODOSC 5MHZ Digital Oscillator as clock source
   * Use default clock divider/pre-divider of 1
   * Use Battery monitor and Battery Monitor internal channels
   */
  ADC12_B_initParam initParam;
  initParam.sampleHoldSignalSourceSelect = ADC12_B_SAMPLEHOLDSOURCE_SC;
  initParam.clockSourceSelect            = ADC12_B_CLOCKSOURCE_ACLK;
  initParam.clockSourceDivider           = ADC12_B_CLOCKDIVIDER_1;
  initParam.clockSourcePredivider        = ADC12_B_CLOCKPREDIVIDER__1;
  initParam.internalChannelMap           = ADC12_B_TEMPSENSEMAP;
  //  initParam.internalChannelMap           = ADC12_B_BATTMAP;
  ADC12_B_init(ADC12_B_BASE, &initParam);

  // Enable the ADC12B module
  ADC12_B_enable(ADC12_B_BASE);

  // Sets up the sampling timer pulse mode
  ADC12_B_setupSamplingTimer(ADC12_B_BASE,
                             ADC12_B_CYCLEHOLD_128_CYCLES,
                             ADC12_B_CYCLEHOLD_128_CYCLES,
                             ADC12_B_MULTIPLESAMPLESDISABLE);

  // Maps Temperature Sensor input channel to Memory 0 and select voltage references
  /*
   * Base address of the ADC12B Module
   * Configure memory buffer 0
   * Map input A1 to memory buffer 0
   * Vref+ = IntBuffer
   * Vref- = AVss
   * Memory buffer 0 is not the end of a sequence
   */
  ADC12_B_configureMemoryParam configureMemoryParam;
  configureMemoryParam.memoryBufferControlIndex = ADC12_B_MEMORY_0;
  configureMemoryParam.inputSourceSelect        = ADC12_B_INPUT_TCMAP;
  //  configureMemoryParam.inputSourceSelect        = ADC12_B_INPUT_BATMAP;
  configureMemoryParam.refVoltageSourceSelect   = ADC12_B_VREFPOS_INTBUF_VREFNEG_VSS;
  configureMemoryParam.endOfSequence            = ADC12_B_NOTENDOFSEQUENCE;
  configureMemoryParam.windowComparatorSelect   = ADC12_B_WINDOW_COMPARATOR_DISABLE;
  configureMemoryParam.differentialModeSelect   = ADC12_B_DIFFERENTIAL_MODE_DISABLE;
  ADC12_B_configureMemory(ADC12_B_BASE, &configureMemoryParam);

  // Clear memory buffer 0 interrupt
  ADC12_B_clearInterrupt(ADC12_B_BASE,
                         0,
                         ADC12_B_IFG0
                         );

  // Enable memory buffer 0 interrupt
  ADC12_B_enableInterrupt(ADC12_B_BASE,
                          ADC12_B_IE0,
                          0,
                          0);

  // Configure internal reference
  while(Ref_A_isRefGenBusy(REF_A_BASE));              // If ref generator busy, WAIT
  Ref_A_enableTempSensor(REF_A_BASE);
  Ref_A_setReferenceVoltage(REF_A_BASE, REF_A_VREF2_0V);
  Ref_A_enableReferenceVoltage(REF_A_BASE);
}

uint16_t readPowerVoltage(void)
{
  // mesure tension
  //Enable/Start sampling and conversion
  /*
   * Base address of ADC12B Module
   * Start the conversion into memory buffer 0
   * Use the single-channel, single-conversion mode
   */
  ADC12_B_startConversion(ADC12_B_BASE,
                          ADC12_B_MEMORY_0,
                          ADC12_B_SINGLECHANNEL);

  while(adc_conv_ready != 1);
  adc_conv_ready = 0;

  return ADC12_B_getResults(ADC12_B_BASE, ADC12_B_MEMORY_0);
}

uint16_t readPowerVoltage_simple(void)
{
  // DMA2CTL |= DMAEN;
  // // ADC12CTL0 |= ADC12ON;
  // // __bis_SR_register(GIE);
  // ADC12CTL0 |= ADC12ENC | ADC12SC;        // enable ADC and start conversion
  // // while(ADC12CTL1 & ADC12BUSY);
  // // __bis_SR_register(CPUOFF);
  // // __bic_SR_register(GIE);
  // while((DMA2CTL & DMAIFG) == 0);
  // DMA2CTL &= ~DMAIFG;
  // DMAIV &= ~BIT2;                         // reset DMAIV interrupt
  // adc_conv_ready = 0;

  // DMA2CTL &= ~DMAEN;
  // ADC12CTL0 &= ~(ADC12ENC);
  // ADC12CTL0 &= ~(ADC12ON);
  // REFCTL0 &= ~REFON;


  ADC12IFGR0 = 0;
	ADC12CTL0 |= ADC12ON;
	ADC12CTL0 &= ~ADC12ENC;
	ADC12CTL0 |= ADC12ENC | ADC12SC;
	while ( !ADC12IFGR0 ) {}
	uint16_t adcResult = ADC12MEM0;
	ADC12CTL0 &= ~ADC12ENC;
	ADC12CTL0 &= ~ADC12ON;
  REFCTL0 &= ~REFON;
	return adcResult;
}

#define APP_COMMON_STOP_SEC_CODE
#include "tpl_memmap.h"

// /*----------------------------------------------------------------------------*/
// /* ISR ADC12                                                                  */
// /*----------------------------------------------------------------------------*/
// #define APP_ISR_handler_adc_end_conversion_START_SEC_CODE
// #include "tpl_memmap.h"
// ISR(handler_adc_end_conversion)
// {
//   adc_conv_ready = 1;
//   ADC12_B_clearInterrupt(ADC12_B_BASE, 0, ADC12_B_IFG0);

// }
// #define APP_ISR_handler_adc_end_conversion_STOP_SEC_CODE
// #include "tpl_memmap.h"
#endif //WITH_CHECKPOINTING
