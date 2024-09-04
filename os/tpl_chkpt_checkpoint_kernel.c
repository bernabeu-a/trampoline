/**
 * @file tpl_os_checkpoint_kernel.c
 *
 * @section descr File description
 *
 * Trampoline checkpointing os services implementation.
 *
 * @section copyright Copyright
 *
 * Trampoline RTOS
 *
 * Trampoline is copyright (c) CNRS, University of Nantes, Ecole Centrale de Nantes
 * Trampoline is protected by the French intellectual property law.
 *
 * This software is distributed under the GNU Public Licence V2.
 * Check the LICENSE file in the root directory of Trampoline
 *
 * @section infos File informations
 *
 * $Date$
 * $Rev$
 * $Author$
 * $URL$
 *
 */

#if WITH_CHECKPOINTING == YES

#include "tpl_os_os_kernel.h"
#include "tpl_os_kernel.h"
#include "tpl_os_definitions.h"
#include "tpl_os_error.h"
#include "tpl_machine_interface.h"
#include "tpl_trace.h"
#include "tpl_os_hooks.h"
#include "tpl_chkpt_checkpoint_kernel.h"
#include "tpl_chkpt_adc.h"

#include "msp430.h"
#include <stdint.h>

#if WITH_RESURRECT == YES
#include "tpl_resurrect_kernel.h"
#ifdef LoRa
#include "rfm9x.h"
#endif
#endif /* WITH_RESURRECT */

#if NUMBER_OF_CORES > 1
#include "tpl_os_multicore_kernel.h"
# if SPINLOCK_COUNT > 0
# include "tpl_as_spinlock_kernel.h"
# endif
#endif
#if WITH_SEQUENCING == YES
#include "tpl_sequence_kernel.h"
#endif

#if WITH_RESURRECT == YES
#include "tpl_resurrect_kernel.h"
#endif

#if WITH_BET == YES
#include "QmathLib.h"
#endif

extern FUNC(void, OS_CODE) tpl_restart_os(void);

#define OS_START_SEC_VAR_NON_VOLATILE_16BIT
#include "tpl_memmap.h"
VAR (sint16,OS_VAR) tpl_checkpoint_buffer = -1;
#define OS_STOP_SEC_VAR_NON_VOLATILE_16BIT
#include "tpl_memmap.h"


#define OS_START_SEC_CODE
#include "tpl_memmap.h"

void init_adc() {
  static uint8_t use1V2Ref = 0;

  ADC12CTL0 &= ~ADC12ENC;                     // disable ADC
  ADC12CTL0 = ADC12ON | ADC12SHT0_2;
  ADC12CTL1 = ADC12SSEL_0 | ADC12DIV_7;       // ADC12OSC as ADC12CLK (~5MHz) / 8
  ADC12CTL3 = ADC12TCMAP | ADC12BATMAP;       // Map Temp and BAT
  ADC12CTL1 |= ADC12SHP;                      // ADCCLK = MODOSC; sampling timer
  ADC12CTL2 |= ADC12RES_2;                    // 12-bit resolution
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
  ADC12CTL0 |= ADC12ENC;
	return;
}

uint16 tpl_ADC_read(){
	ADC12CTL0 |= ADC12SC;
	while(!(ADC12IFGR0 & BIT0));
	// ADC12CTL0 &= ~(ADC12ENC);
	// REFCTL0 &= ~(REFON);
	return ADC12MEM0;
}

void tpl_RTC_init()
{

    // Unlock RTC key protected registers
    RTCCTL0_H = RTCKEY_H;
    // Clear RTCAIE RTCAIFG and AE bits to prevent potential erroneous alarm condition
    RTCCTL0_L &= ~RTCAIE;
    RTCCTL0_L &= ~RTCAIFG;
    // Calendar + Hold, Hexa code
    RTCCTL1 = RTCHOLD | RTCMODE;
    // Random year, day, month, day of week, hour, set minute and sec to 0
    RTCYEAR = 0x0000;
    RTCMON = 0x0;
    RTCDAY = 0x00;
    RTCDOW = 0x00;
    RTCHOUR = 0x0;
    RTCMIN = 0x00;
    RTCSEC = 40;
    // reset all register alarm because they are in undefined state when reset
    RTCAHOUR &= ~(0x80);
    RTCADOW &= ~(0x80);
    RTCADAY &= ~(0x80);
    // set alarm in next '1' minutes + enable interrupt
    #define PERIOD_WAKE_UP 0x01
    RTCAMIN = (PERIOD_WAKE_UP | (1<<7));
    // Interrupt from alarm
    RTCCTL0_L = RTCAIE;
    // RTCCTL0_H = 0x0;
    // start rtc
    RTCCTL1 &= ~(RTCHOLD);
}

 void tpl_RTC_init_time(uint8_t hibernate_time_second, uint8_t hibernate_time_minute){
    // Unlock RTC key protected registers
    RTCCTL0_H = RTCKEY_H;
    // Clear RTCAIE RTCAIFG and AE bits to prevent potential erroneous alarm condition
    RTCCTL0_L &= ~RTCAIE;
    RTCCTL0_L &= ~RTCAIFG;
    // Calendar + Hold, Hexa code
    RTCCTL1 = RTCHOLD | RTCMODE;
    // Random year, day, month, day of week, hour, set minute and sec to 0
    RTCYEAR = 0x0000;
    RTCMON = 0x0;
    RTCDAY = 0x00;
    RTCDOW = 0x00;
    RTCHOUR = 0x0;
    RTCMIN = 0x00;
    // reset all register alarm because they are in undefined state when reset
    RTCAHOUR &= ~(0x80);
    RTCADOW &= ~(0x80);
    RTCADAY &= ~(0x80);
    if(hibernate_time_minute!=0){
        if(hibernate_time_second!=0){
            RTCSEC = 60-hibernate_time_second;
            RTCAMIN = ((hibernate_time_minute+1) | (1<<7));
        }
        else{
            RTCSEC = 0x00;
            RTCAMIN = (hibernate_time_minute | (1<<7));
        }
    }
    else{
        RTCSEC = 60 - hibernate_time_second;
        RTCAMIN = ((RTCMIN + 1) | (1<<7));
    }
    // Interrupt from alarm
    RTCCTL0_L = RTCAIE;
    // RTCCTL0_H = 0x0;
    // start rtc
    RTCCTL1 &= ~(RTCHOLD);
 }


void tpl_RTC_stop()
{
  // stop rtc
  RTCCTL1 |= RTCHOLD;
}

uint8_t tpl_lpm_hibernate()
{
  /*
    Disable TIMER3_A0 interrupt
    in LPM3, ACLK is still active
  */
  // TA3CCTL0 &= ~CCIE;
  /* enters in LPM - enable interrupt for RTC*/
  __bis_SR_register(LPM3_bits + GIE);
  /* remove interrupts (we are in kernel mode) */
  __bic_SR_register(GIE);
  /* Restore TIMER3_A0 interrupt */
  // TA3CCTL0 |= CCIE;
  return 1;
}

FUNC(void, OS_CODE) tpl_chkpt_hibernate(){
  /* Disable Microphone if with BARD */
  #ifdef BARD
  P4OUT &= ~BIT7;
  #endif
  P2VAR(uint16_t, AUTOMATIC, OS_VAR) result_adc_adc = &result_adc;
  // P1OUT |= BIT4;
  sint16 l_buffer;
  uint8_t was_sleeping = 0;
  l_buffer = (tpl_checkpoint_buffer + 1) % 2;
  tpl_save_checkpoint(l_buffer);
  /* Avoid using DMA with ADC linked to DMA */
  // tpl_save_checkpoint_dma(l_buffer);
  tpl_checkpoint_buffer = l_buffer;

  uint16_t waiting_loop = 1;

  while(waiting_loop){
    /* Get energy level from ADC */
    bool use1V2Ref = true;
    tpl_adc_init_simple(use1V2Ref, result_adc_adc);
    /* Polling */
    result_adc = readPowerVoltage_simple();
    // uint16_t energy = (~result_adc)+1;
    uint16_t energy = result_adc;
    uint16_t voltageInMillis;
    if(energy == 0x0FFF){
      use1V2Ref = false;
      tpl_adc_init_simple(use1V2Ref, result_adc_adc);
      result_adc = readPowerVoltage_simple();
      // energy = (~result_adc)+1;
      energy = result_adc;
      voltageInMillis = energy;
    }
    else{
      voltageInMillis = energy*3/5;
    }
    #if WITH_RESURRECT == YES
    P2VAR(tpl_step, AUTOMATIC, OS_VAR) tmp_ptr_step = NULL;
    CONSTP2CONST(tpl_step_ref, AUTOMATIC, OS_VAR)
    ptr_state = tpl_step_state[tpl_kern_resurrect.state];
    uint8_t i;
    uint32 tmp_step_energy = 0xFFFFFFFF;
    for (i = 0; i < ENERGY_LEVEL_COUNT; i++){
        tmp_ptr_step = (P2VAR(tpl_step, AUTOMATIC, OS_VAR))ptr_state[i];
        if(tmp_ptr_step->energy < tmp_step_energy){
          tmp_step_energy = tmp_ptr_step->energy;
        }
    }
    #if WITH_BET == YES
    uint16_t predictionInMillis = 0;
    if(tpl_resurrect_energy.power_prediction != 0){
        uint32_t time_tmp_step = 0;
        for(i=0; i<tmp_ptr_step->activity->nb_activity; i++){
            time_tmp_step += tmp_ptr_step->activity->time_activity[i];
        }
        #ifdef debug_bet
        // tpl_serial_print_string("time: ");
        // tpl_serial_print_int(time_tmp_step, 0);
        // tpl_serial_print_string("\n");
        #endif
        float prediction_from_power = (float) ((float)tpl_resurrect_energy.power_prediction * (float) time_tmp_step);
        /* We have nJ, so with nF as capacitance, we have V */
        float prediction_v2 = (2 * prediction_from_power) / 6800000.0;
        /* We have prediction_v2 as v^2 -> to q12 for division and sqrt */
        _q12 prediction_v = _Q12sqrt(_Q12(prediction_v2));
        /* Cast prediction to float to convert into mV */
        float prediction_v_float = (_Q12toF(prediction_v)) * 1000;
        predictionInMillis = (uint16_t) prediction_v_float;
        #ifdef debug_bet
        // #define FLOAT_TO_INT(x) ((x)>=0?(int16_t)((x)+0.5):(int16_t)((x)-0.5))
        // tpl_serial_print_string("pred_chkpt: ");
        // tpl_serial_print_int(FLOAT_TO_INT((float)prediction_v_float), 0);
        tpl_serial_print_string("vpred: ");
        tpl_serial_print_int(predictionInMillis, 0);
        tpl_serial_print_string("\n");
        tpl_serial_print_string("vmilli: ");
        tpl_serial_print_int(voltageInMillis, 0);
        tpl_serial_print_string("\n");
        // tpl_serial_print_string("vstep");
        // tpl_serial_print_int(tmp_step_energy, 0);
        // tpl_serial_print_string("\n");
        #endif
    }
    if(voltageInMillis + predictionInMillis > tmp_step_energy)
    #else
    if(voltageInMillis > tmp_step_energy)
    #endif /* WITH_ENERGY_PREDICTION & WITH_BET */
    {
        tpl_RTC_stop();
		waiting_loop = 0;
	}
	else{
        if(was_sleeping != 1){
            #ifdef LoRa
            setModeSleep();
            // setModeIdle();
            // wait 400/500 µS
            for (volatile uint32_t i = 0; i < 400000; i++);
            // Remove MOSI - MISO - CLK - CS of LoRa

            P6SEL0 &= ~(BIT0 + BIT1 + BIT2);               // P6.0 = MOSI, P6.1 = MISO, P6.2 = CLK,
            P6SEL1 &= ~(BIT0 + BIT1 + BIT2);               // P6.0 = MOSI, P6.1 = MISO, P6.2 = CLK
            P6DIR |= (BIT0 + BIT1 + BIT2);
            P6OUT &= ~(BIT0 + BIT1 + BIT2);

            P3DIR  |= BIT0;                                // Set P3.0 as OUTPUT for CS
            P3OUT  &= ~BIT0;                               // Set P3.0 LOW
            // Remove power supply of LoRa
            P5DIR |= BIT7;
            P3DIR &= ~BIT1;
            P5OUT &= ~BIT7;
            /* Disable GPIO power-on default high-impedance mode for FRAM devices */
            PM5CTL0 &= ~LOCKLPM5;
            for (volatile uint32_t i = 0; i < 400000; i++);
            #endif
        }

        #if WITH_BET == YES
        if(tpl_resurrect_energy.power_prediction != 0){
            /* Use prediction to compute hibernation time */
            /* hibernate pred is in ms */
            int32_t diff_v = tmp_step_energy - voltageInMillis;
            #ifdef debug_bet
            // tpl_serial_print_string("step: ");
            // tpl_serial_print_int(tmp_step_energy, 0);
            // tpl_serial_print_string("\n");
            // tpl_serial_print_string("vcur: ");
            // tpl_serial_print_int(voltageInMillis, 0);
            // tpl_serial_print_string("\n");
            #endif
            int32_t diff_v2 = diff_v * diff_v;
            // float hibernate_pred = ((((float)tmp_step_energy/1000.0)) - (((float)voltageInMillis/1000.0))) * ((((float)tmp_step_energy/1000.0)) - (((float)voltageInMillis/1000.0)));
            // hibernate_pred *= hibernate_pred;
            float hibernate_pred = ((float)(diff_v2 / 1000) * 6800000.0)/(2*(float)tpl_resurrect_energy.power_prediction) ;
            /* Compute minutes and second for RTC setup */
            // float hibernate_time_us = (float)tmp_step_energy - (float)voltageInMillis/(float)tpl_resurrect_energy.power_prediction;
            uint32_t hibernate_time_second = (uint32_t)(hibernate_pred / 1000.0);
            #ifdef debug_bet
            tpl_serial_print_string("hib_pred: ");
            tpl_serial_print_int(hibernate_pred, 0);
            tpl_serial_print_string("\n");
            tpl_serial_print_string("hibern: ");
            tpl_serial_print_int(hibernate_time_second, 0);
            tpl_serial_print_string("\n");
            #endif
            uint8_t hibernate_time_minute = 0;
            while(hibernate_time_second > 60){
                hibernate_time_minute++;
                hibernate_time_second = hibernate_time_second - 60;
            }
            hibernate_time_second = (uint8_t) hibernate_time_second;
            #ifdef debug_bet
            // tpl_serial_print_string("s: ");
            // tpl_serial_print_int(hibernate_time_second,0);
            // tpl_serial_print_string("\n");
            // tpl_serial_print_string("m: ");
            // tpl_serial_print_int(hibernate_time_minute,0);
            // tpl_serial_print_string("\n");
            for (volatile uint32_t i = 0; i < 400000; i++);
            for (volatile uint32_t i = 0; i < 400000; i++);
            for (volatile uint32_t i = 0; i < 400000; i++);
            // for (volatile uint32_t i = 0; i < 400000; i++);
            // for (volatile uint32_t i = 0; i < 400000; i++);
            // for (volatile uint32_t i = 0; i < 400000; i++);

            #endif
            tpl_RTC_init_time(hibernate_time_second, hibernate_time_minute);
        }
        else{
            tpl_RTC_init(); //startRTC => interrupt next 1 min
        }
        #else
        tpl_RTC_init(); //startRTC => interrupt next 1 min
        #endif  /* WITH_BET */
        #ifndef BARD
        P7OUT |= BIT0;
        #endif
        was_sleeping = tpl_lpm_hibernate();
        #ifndef BARD
        P7OUT &= ~BIT0;
        #endif
    }
    #endif /* WITH_RESURRECT == YES */
    #if WITH_RESURRECT == NO
    if(voltageInMillis > RESUME_FROM_HIBERNATE_THRESHOLD){
      tpl_RTC_stop();
		  waiting_loop = 0;
	  }
	else{
        if(was_sleeping == 1){

        }
        else{
            #ifdef LoRa
            setModeSleep();
            // setModeIdle();
            // wait 400/500 µS
            for (volatile uint32_t i = 0; i < 400000; i++);
            // Remove MOSI - MISO - CLK - CS of LoRa
            P6SEL0 &= ~(BIT0 + BIT1 + BIT2);               // P6.0 = MOSI, P6.1 = MISO, P6.2 = CLK,
            P6SEL1 &= ~(BIT0 + BIT1 + BIT2);               // P6.0 = MOSI, P6.1 = MISO, P6.2 = CLK
            P6DIR |= (BIT0 + BIT1 + BIT2);
            P6OUT &= ~(BIT0 + BIT1 + BIT2);

            P3DIR  |= BIT0;                                // Set P3.0 as OUTPUT for CS
            P3OUT  &= ~BIT0;                               // Set P3.0 LOW

            // Remove power supply of LoRa
            P5DIR |= BIT7;
            P3DIR &= ~BIT1;
            P5OUT &= ~BIT7;
            /* Disable GPIO power-on default high-impedance mode for FRAM devices */
            PM5CTL0 &= ~LOCKLPM5;
            for (volatile uint32_t i = 0; i < 400000; i++);
            #endif
        }
      tpl_RTC_init(); //startRTC => interrupt next 1 min
      #ifndef BARD
      P7OUT |= BIT0;
      #endif
      was_sleeping = tpl_lpm_hibernate();
      #ifndef BARD
      P7OUT &= ~BIT0;
      #endif
  }
    #endif /* WITH_RESURRECT == NO */
  }
    /* If use BET, reset prediction and variance buffer */
    #if WITH_BET == YES
    uint8_t index;

    // for(index=0; index<tpl_resurrect_energy.previous_harvesting->current_size; index++){
    //     tpl_resurrect_energy.previous_harvesting->buffer[index] = 0;
    // }

    for(index=0; index<tpl_resurrect_energy.power_previous_harvesting->current_size; index++){
        tpl_resurrect_energy.power_previous_harvesting->buffer[index] = 0;
    }

    for(index=0; index<tpl_kern_resurrect.variance_buffer->current_size; index++){
        tpl_kern_resurrect.variance_buffer->buffer[index] = 0;
    }
    // tpl_resurrect_energy.previous_harvesting->current_size = 0;
    tpl_resurrect_energy.power_previous_harvesting->current_size = 0;
    tpl_kern_resurrect.variance_buffer->current_size = 0;

    // tpl_resurrect_energy.previous_harvesting->index = 0;
    tpl_resurrect_energy.power_previous_harvesting->index = 0;
    tpl_kern_resurrect.variance_buffer->index = 0;

    /* We set wake up to one, to avoid doing a prediction for the next step */
    tpl_resurrect_energy.wake_up = TRUE;
    /* And we reset award accumulated */
    tpl_kern_resurrect.award = 0;

    #ifdef debug_bet
    tpl_serial_print_string("out\n");
    #endif
    #endif

    // Setup IO and LoRa when exiting
    #ifdef LoRa
    setupIO();

    /* LoRa */
    writeRegister(RH_RF95_REG_01_OP_MODE | RH_SPI_WRITE_MASK, RH_RF95_MODE_SLEEP | RH_RF95_LONG_RANGE_MODE);

    long wait = 32600;
    long w;
    for(w=0; w<wait; w++){
    }
    if(readRegister(RH_RF95_REG_01_OP_MODE) != (RH_RF95_MODE_SLEEP | RH_RF95_LONG_RANGE_MODE)){
        //tpl_serial_print_string("error");
    }
    writeRegister(RH_RF95_REG_0E_FIFO_TX_BASE_ADDR, 0);
    writeRegister(RH_RF95_REG_0F_FIFO_RX_BASE_ADDR, 0);
    setModeIdle();

    setPreambleLength(8);
    setFrequency(868.0);
    setTxPower(5);
    #endif
}

FUNC(void, OS_CODE) tpl_hibernate_os_service(void)
{
  GET_CURRENT_CORE_ID(core_id)

  /* init the error to no error */
  VAR(StatusType, AUTOMATIC) result = E_OK;

  /* lock the kernel */
  LOCK_KERNEL()
  /* store information for error hook routine */
  STORE_SERVICE(OSServiceId_TerminateTask)
  /* check interrupts are not disabled by user */
  CHECK_INTERRUPT_LOCK(result)
  /* check we are at the task level */
  CHECK_TASK_CALL_LEVEL_ERROR(core_id,result)
  /* check the task does not own a resource */
  CHECK_RUNNING_OWNS_REZ_ERROR(core_id, result)
  /*  checks the task does not occupy spinlock(s)   */
  CHECK_SCHEDULE_WHILE_OCCUPED_SPINLOCK(core_id, result)

#if TASK_COUNT > 0
  IF_NO_EXTENDED_ERROR(result)
  {
    /* the activate count is decreased */
    TPL_KERN(core_id).running->activate_count--;
    /* terminate the running task */
    tpl_terminate();
    /* task switching should occur */
    TPL_KERN(core_id).need_switch = NEED_SWITCH;
    /* start the highest priority process */
    tpl_start(CORE_ID_OR_NOTHING(core_id));

    SWITCH_CONTEXT_NOSAVE(CORE_ID_OR_NOTHING(core_id))
  }
#endif

  PROCESS_ERROR(result)

  /*  unlock the kernel  */
  UNLOCK_KERNEL()
  tpl_chkpt_hibernate();
  PROCESS_ERROR(result)
}

#if __GXX_ABI_VERSION == 1011 || __GXX_ABI_VERSION == 1013
/* ISR1 related to RTC_VECTOR */
__attribute__((section (".irq_func")))
__interrupt void tpl_direct_irq_handler_RTC_VECTOR(void)
#elif __GXX_ABI_VERSION == 1002
void __attribute__((interrupt(RTC_VECTOR))) tpl_direct_irq_handler_RTC_VECTOR()
#else
    #error "Unsupported ABI"
#endif
{
  // Clear interrupt flag
  RTCCTL0_L &= ~RTCAIFG;
  // Exit LPM
  LPM3_EXIT;
}

FUNC(void, OS_CODE) tpl_restart_os_service(void)
{

  // P1OUT &= ~BIT5;
  GET_CURRENT_CORE_ID(core_id)

    //#if (WITH_ERROR_HOOK == YES) || (WITH_OS_EXTENDED == YES) | (WITH_ORTI == YES)
  /*  init the error to no error  */
    //  VAR(tpl_status, AUTOMATIC) result = E_OK;
    //#endif

  /*  store information for error hook routine    */
  STORE_SERVICE(OSServiceId_StartOS)
  STORE_MODE(mode);

  //  CHECK_OS_NOT_STARTED(core_id, result)

  //  IF_NO_EXTENDED_ERROR(result)
  //  {
  //#if NUMBER_OF_CORES > 1
    /*
     * Sync barrier at start of tpl_start_os_service.
     */
  //    tpl_sync_barrier(&tpl_start_count_0, &tpl_startos_sync_lock);
  //#endif

  //    TRACE_TPL_INIT()

      //    tpl_init_os(mode);

      //    tpl_enable_counters();

    /*
     * Call the startup hook. According to the spec, it should be called
     * after the os is initialized and before the scheduler is running
     */
      //    CALL_STARTUP_HOOK()

    /*
     * Call the OS Application startup hooks if needed
     */
      //    CALL_OSAPPLICATION_STARTUP_HOOKS()

  //#if NUMBER_OF_CORES > 1
    /*
     * Sync barrier just before starting the scheduling.
     */
  //    tpl_sync_barrier(&tpl_start_count_1, &tpl_startos_sync_lock);
  //#endif

    /*
     * Call tpl_start_scheduling to elect the highest priority task
     * if such a task exists.
     */

  // P1OUT |= BIT5;
  tpl_load_checkpoint(tpl_checkpoint_buffer);
  /* Avoid using DMA with ADC linked to DMA */
  // tpl_load_checkpoint_dma(tpl_checkpoint_buffer);
  // P1OUT &= ~BIT5;
  #if WITH_SEQUENCING == YES
    tpl_choose_next_sequence();
    tpl_start(CORE_ID_OR_NOTHING(core_id));
  #endif

  #if WITH_RESURRECT == YES

    #if WITH_ENERGY_PREDICTION & WITH_BET == 1
    uint8_t index;

    for(index=0; index<tpl_resurrect_energy.previous_harvesting->current_size; index++){
        tpl_resurrect_energy.previous_harvesting->buffer[index] = 0;
    }
    for(index=0; index<tpl_kern_resurrect.variance_buffer->current_size; index++){
        tpl_kern_resurrect.variance_buffer->buffer[index] = 0;
    }
    tpl_resurrect_energy.previous_harvesting->current_size = 0;
    tpl_kern_resurrect.variance_buffer->current_size = 0;

    tpl_resurrect_energy.previous_harvesting->index = 0;
    tpl_kern_resurrect.variance_buffer->index = 0;

    /* We set wake up to one, to avoid doing a prediction for the next step */
    tpl_resurrect_energy.wake_up = TRUE;
    #endif

    // P1OUT &= ~BIT5;
    tpl_choose_next_step();
    // tpl_start(CORE_ID_OR_NOTHING(core_id));
    tpl_start(CORE_ID_OR_NOTHING(core_id));
    SWITCH_CONTEXT_NOSAVE(CORE_ID_OR_NOTHING(core_id))
  #endif

    /* Posix */
  //    SWITCH_CONTEXT_NOSAVE(core_id)
      //  }

  PROCESS_ERROR(result)
}

#define OS_STOP_SEC_CODE
#include "tpl_memmap.h"

FUNC(void, OS_CODE) RestartOS(void)
{
  GET_CURRENT_CORE_ID(core_id)

  tpl_init_machine();
#if WITH_MODULES_INIT == YES
  /* Call inits for kernel modules. Generated function */
  tpl_init_modules();
#endif
  tpl_restart_os();
  /*
   * tpl_start_os does not return since the idle task will run
   * if no other task is AUTOSTART except if it is called from
   * within a task or an ISR of (which is forbidden).
   */
}

/* End of file tpl_os_checkpoint_kernel.c */

#endif //WITH_CHECKPOINTING
