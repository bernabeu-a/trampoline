/**
 * @file tpl_resurrect_kernel.c
 *
 * @section descr File description
 *
 * Trampoline sequence os services implementation.
 *
 * @section copyright Copyright
 *
 * Trampoline RTOS
 *
 * Trampoline is copyright (c) CNRS, University of Nantes, Ecole Centrale de
 * Nantes Trampoline is protected by the French intellectual property law.
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

#include "tpl_resurrect_kernel.h"
#include "tpl_chkpt_checkpoint_kernel.h"
#include "tpl_machine_interface.h"
#include "tpl_os_alarm_kernel.h"
#include "tpl_os_definitions.h"
#include "tpl_os_error.h"
#include "tpl_os_hooks.h"
#include "tpl_os_kernel.h"
#include "tpl_os_os_kernel.h"
#include "tpl_trace.h"

#include "msp430.h"

#include "tpl_chkpt_adc.h"

#include <stdint.h>

#if WITH_RESURRECT == YES
#include "tpl_resurrect_kernel.h"

#define OS_START_SEC_VAR_UNSPECIFIED
#include "tpl_memmap.h"
STATIC VAR(tpl_application_mode, OS_VAR) application_mode_step = NOAPPMODE;
#define OS_STOP_SEC_VAR_UNSPECIFIED
#include "tpl_memmap.h"

#define OS_START_SEC_VAR_NON_VOLATILE_16BIT
#include "tpl_memmap.h"
VAR(uint16_t, AUTOMATIC) result_adc=0;
#define OS_STOP_SEC_VAR_NON_VOLATILE_16BIT
#include "tpl_memmap.h"


FUNC(void, OS_CODE)
tpl_init_resurrect_os(CONST(tpl_application_mode, AUTOMATIC) app_mode)
{
  GET_CURRENT_CORE_ID(core_id);
  // VAR(uint16, AUTOMATIC) i;
  VAR(tpl_status, AUTOMATIC) result = E_OK;
  /*  Start the idle task */
#if NUMBER_OF_CORES == 1
  result = tpl_activate_task(IDLE_TASK_ID);
#else
  result = tpl_activate_task(IDLE_TASK_0_ID + tpl_get_core_id());
#endif

  /*  Init the Ioc's unqueued buffer */
#if ((WITH_IOC == YES) && (IOC_UNQUEUED_COUNT > 0))
  {
    tpl_ioc_init_unqueued();
  }
#endif

  tpl_choose_next_step();
}

FUNC(void, OS_CODE)
tpl_start_os_resurrect_service(CONST(tpl_application_mode, AUTOMATIC) mode)
{
  GET_CURRENT_CORE_ID(core_id)
  tpl_init_machine();
#if WITH_MODULES_INIT == YES
  tpl_init_modules();
#endif
#if (WITH_ERROR_HOOK == YES) || (WITH_OS_EXTENDED == YES) | (WITH_ORTI == YES)
  /*  init the error to no error  */
  VAR(tpl_status, AUTOMATIC) result = E_OK;
#endif
  /*  lock the kernel    */
  LOCK_KERNEL()

  /*  store information for error hook routine    */
  STORE_SERVICE(OSServiceId_StartResurrect)
  STORE_MODE(mode)

  CHECK_OS_NOT_STARTED(core_id, result)

  IF_NO_EXTENDED_ERROR(result)
  {
    application_mode_step = mode;

    tpl_init_resurrect_os(mode);

    tpl_enable_counters();

    /*
     * Call the startup hook. According to the spec, it should be called
     * after the os is initialized and before the scheduler is running
     */
    CALL_STARTUP_HOOK()

    /*
     * Call the OS Application startup hooks if needed
     */
    CALL_OSAPPLICATION_STARTUP_HOOKS()

    /*
     * Call tpl_start_scheduling to elect the highest priority task
     * if such a task exists.
     */

    tpl_start_scheduling(CORE_ID_OR_NOTHING(core_id));

    SWITCH_CONTEXT_NOSAVE(core_id)
  }

  PROCESS_ERROR(result)

  /*  unlock the kernel  */
  UNLOCK_KERNEL()
}

FUNC(void, OS_CODE) tpl_choose_next_step(void){
    // VAR(StatusType, AUTOMATIC) result = E_OK;

    VAR(uint16, AUTOMATIC) i;
    CONSTP2CONST(tpl_step_ref, AUTOMATIC, OS_VAR)
    ptr_state = tpl_step_state[tpl_kern_resurrect.state];
    P2VAR(tpl_step, AUTOMATIC, OS_VAR) ptr_step = NULL;
    P2VAR(tpl_step, AUTOMATIC, OS_VAR) tmp_ptr_step = NULL;

    /* We compute energy consumed if using TIMER_ACTIVITY */
    #if WITH_TIMER_ACTIVITY
    uint8_t k;
    uint32_t voltage_harvested = 0;
    uint32_t voltage_worst_case = (uint32_t)tpl_kern_resurrect.energy_at_start*1000;
    uint32_t voltage_consumed = (uint32_t)tpl_kern_resurrect.energy_at_start*1000;
    /* Slope are in µV per ms and time is in ms*/
    if(tpl_kern_resurrect.elected != NULL){
        for(k=0; k<tpl_kern_resurrect.elected->activity->nb_activity; k++){
        voltage_worst_case -= ((uint32_t)tpl_kern_resurrect.elected->activity->slope[k] * (uint32_t)tpl_kern_resurrect.elected->activity->time_activity[k]);
        voltage_consumed -= ((uint32_t)tpl_kern_resurrect.elected->activity->slope[k] * (uint32_t)tpl_kern_resurrect.elected->activity->current_time_activity[k]);
        }
    }
    #endif /* WITH_TIMER_ACTIVITY */

    P2VAR(uint16_t, AUTOMATIC, OS_VAR) result_adc_adc = &result_adc;
    while (ptr_step == NULL)
    {
      // P1OUT |= BIT2;
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

      /* Compute energy harvested (in µV) */
      #if WITH_TIMER_ACTIVITY
      /* Check if startup or not */
      if(tpl_kern_resurrect.elected != NULL){
          voltage_harvested = (((uint32_t)voltageInMillis)*1000 - voltage_worst_case) - (voltage_consumed - voltage_worst_case) ;
      }
      #endif /* WITH_TIMER_ACTIVITY */
      #if WITH_ENERGY_PREDICTION
      if(tpl_kern_resurrect.elected != NULL){
        const uint16_t index = (tpl_resurrect_energy.previous_harvesting->index++) % SMA_COUNT;
        tpl_resurrect_energy.previous_harvesting->buffer[index] = voltage_harvested;
        if(tpl_resurrect_energy.previous_harvesting->current_size != SMA_COUNT){
            tpl_resurrect_energy.previous_harvesting->current_size++;
        }
        if(tpl_resurrect_energy.previous_harvesting->index == SMA_COUNT){
            tpl_resurrect_energy.previous_harvesting->index = 0;
        }
        tpl_resurrect_energy.prediction = tpl_prediction_sma();
      }
      #endif /* WITH_ENERGY_PREDICTION */
      for (i = 0; i < ENERGY_LEVEL_COUNT; i++)
      {
        tmp_ptr_step = (P2VAR(tpl_step, AUTOMATIC, OS_VAR))ptr_state[i];
        #if WITH_ENERGY_PREDICTION
        if (voltageInMillis + (uint16_t)(tpl_resurrect_energy.prediction >> 3))
        #else
        if (voltageInMillis >= tmp_ptr_step->energy)
        #endif /* WITH_ENERGY_PREDICTION */
        {
          #if WITH_RESURRECT_EVENT == YES
          VAR(uint8, AUTOMATIC) j;
          VAR(uint16, AUTOMATIC) mask = 0x0001;
          for(j = 0; j < RESURRECT_EVENT_COUNT; j++){
            if(((tpl_kern_resurrect.state_event & mask)>>j) == (tmp_ptr_step->resurrect_event[j])){
              mask = mask<<1;
            }
            else{
              break;
            }
            tpl_kern_resurrect.elected = tmp_ptr_step;
            ptr_step = tmp_ptr_step;
          }
          #else
          tpl_kern_resurrect.elected = tmp_ptr_step;
          ptr_step = tmp_ptr_step;
          break;
          #endif /* WITH_RESURRECT_EVENT */
        }
      }
      /* Not enough energy to elect next step --> hibernate */
      if (ptr_step == NULL)
      {
        tpl_chkpt_hibernate();
      }
      /* Save Energy at start if starting a step */
      #if WITH_TIMER_ACTIVITY
      tpl_kern_resurrect.energy_at_start = voltageInMillis;
      #endif /* WITH_TIMER_ACTIVITY */
    }

    /* Point the entry point of a the task to the right function */

    CONSTP2VAR(tpl_proc, AUTOMATIC, OS_VAR) resurrect_proc = tpl_dyn_proc_table[RESURRECT_TASK_ID];
    // tpl_proc_static *resurrect_proc = (tpl_proc_static *) the_proc;
    // CONSTP2VAR(tpl_proc_static, AUTOMATIC, OS_VAR) resurrect_proc = (CONSTP2VAR(tpl_proc_static, AUTOMATIC,OS_VAR))the_proc;
    resurrect_proc->entry = (tpl_proc_function)tpl_kern_resurrect.elected->entry_point;
    #if WITH_RESURRECT_EVENT
    /* Reset Event Resurrect */
    for(i = 0; i < RESURRECT_EVENT_COUNT; i++){
      if(tpl_kern_resurrect.elected->resurrect_event[i]==1){
        tpl_kern_resurrect.state_event &= ~(1<<i);
      }
    }
    #endif /* WITH_RESURRECT_EVENT */
    /* Activate the resurrect task */
    tpl_activate_task(RESURRECT_TASK_ID);
    /* Update tpl_kern_resurrect.state with next state from step elected */
    tpl_kern_resurrect.state = tpl_kern_resurrect.elected->to_state;
}


FUNC(void, OS_CODE) tpl_terminate_step_resurrect_service(void){

  GET_CURRENT_CORE_ID(core_id)

  /* init the error to no error */
  VAR(StatusType, AUTOMATIC) result = E_OK;

  /* lock the kernel */
  LOCK_KERNEL()
  /* store information for error hook routine */
  STORE_SERVICE(OSServiceId_TerminateTaskSequence)
  /* check interrupts are not disabled by user */
  CHECK_INTERRUPT_LOCK(result)
  /* check we are at the task level */
  CHECK_TASK_CALL_LEVEL_ERROR(core_id, result)
  /* check the task does not own a ressource */
  CHECK_RUNNING_OWNS_REZ_ERROR(core_id, result)
  /* check the task does not occupy spinlock(s) */
  CHECK_SCHEDULE_WHILE_OCCUPED_SPINLOCK(core_id, result)

#if TASK_COUNT > 0
  IF_NO_EXTENDED_ERROR(result){
    /* the activate count is decreased */
    TPL_KERN(core_id).running->activate_count--;
    /* terminate the running task */
    tpl_terminate();
    /* task switching should occur */
    TPL_KERN(core_id).need_switch = NEED_SWITCH;
  }
#endif

  /* unlock kernel */
  UNLOCK_KERNEL()

  tpl_choose_next_step();

  /* start the highest priority process */
  tpl_start(CORE_ID_OR_NOTHING(core_id));
  SWITCH_CONTEXT_NOSAVE(CORE_ID_OR_NOTHING(core_id))

  PROCESS_ERROR(result)
  return;
}


FUNC(void, OS_CODE) tpl_set_activation_alarm_service(CONST(tpl_alarm_id, AUTOMATIC)  alarm_id, VAR(int, AUTOMATIC) set_nb_activation){
  #if ALARM_COUNT > 0
    P2VAR(tpl_time_obj, AUTOMATIC, OS_APPL_DATA) alarm;
    alarm = tpl_alarm_table[alarm_id];
    alarm->nb_iteration = set_nb_activation;
  #endif

  return;
}
// #if WITH_RESURRECT_EVENT == YES
// FUNC(void, OS_CODE) tpl_set_event_resurrect_service(
//   CONST(EventResurrectType, AUTOMATIC) event_resurrect){

//     tpl_kern_resurrect.state_event |= 1<<event_resurrect;

//     return;
// }

// #endif // WITH_RESURRECT_EVENT
#if WITH_ENERGY_PREDICTION == YES
#if ENERGY_PREDICTOR == SMA
/* Prediction with sliding moving average */
FUNC(uint32_t, OS_CODE) tpl_prediction_sma(void)
{
    uint8_t i;
    uint32_t result = 0;
    for(i=0; i<tpl_resurrect_energy.previous_harvesting->current_size; i++){
        result += tpl_resurrect_energy.previous_harvesting->buffer[i];
    }
    return (result/tpl_resurrect_energy.previous_harvesting->current_size);
}
#endif // ENERGY_PREDICTOR == "SMA"
#endif // WITH_ENERGY_PREDICTION

#endif // WITH_RESURRECT
