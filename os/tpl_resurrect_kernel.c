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

#include "tpl_chkpt_checkpoint_kernel.h"
#include "tpl_machine_interface.h"
#include "tpl_os_definitions.h"
#include "tpl_os_error.h"
#include "tpl_os_hooks.h"
#include "tpl_os_os_kernel.h"
#include "tpl_os_timeobj_kernel.h"
#include "msp430.h"

#include "tpl_chkpt_adc.h"
#include <math.h>

#if WITH_RESURRECT == YES
#include "tpl_resurrect_kernel.h"

#if WITH_BET == YES
#define THESHOLD_AWARD 100 /* A bit random atm */
#include "QmathLib.h"
#include "math.h"
// #define PI      3.1415926536

_q12 gaussian(_q12 mu, _q12 sigma, _q12 x)
{
    float mu_float = _Q12toF(mu);
    float sigma_float = _Q12toF(sigma);
    float x_float = _Q12toF(x);
    float sqrtf_pi = sqrtf(2.0);
    float erff_f = erff((x_float - mu_float) / (sigma_float * sqrtf_pi));
    return (_Q12mpy(_Q12(0.5), _Q12(1+erff_f)));
    // _q12 res;
    // _q12 pi2 = _Q12mpy(_Q12(2), _Q12(PI));
    // _q12 mu_x = _Q12mpy(x - mu, x - mu);
    // _q12 sigma_2 = _Q12mpy(sigma, sigma);
    // _q12 sigma_2_2 = _Q12mpy(sigma_2, _Q12(2));
    // _q12 sigma_sqrt_pi = _Q12mpy(sigma, _Q12sqrt(pi2));
    // _q12 div_mu_sigma = _Q12div(mu_x, sigma_2_2);
    // _q12 tmp_res = _Q12exp(~(div_mu_sigma)+1);
    // res = _Q12div(tmp_res, sigma_sqrt_pi);
    // // res = _Q12div(_Q12exp(-_Q12div(_Q12mpy(mu - x,mu - x), _Q12mpy(sigma, sigma))), _Q12mpy(sigma, _Q12sqrt(pi2)));
    // return res;
}
#endif /* WITH_BET */

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
#if WITH_ENERGY_PREDICTION & WITH_BET == 1
    uint8_t index;

    for(index=0; index<tpl_resurrect_energy.power_previous_harvesting->current_size; index++){
        tpl_resurrect_energy.power_previous_harvesting->buffer[index] = 0;
    }
    for(index=0; index<tpl_resurrect_energy.variance_buffer->current_size; index++){
        tpl_resurrect_energy.variance_buffer->buffer[index] = 0;
    }
    tpl_resurrect_energy.power_previous_harvesting->current_size = 0;
    tpl_resurrect_energy.variance_buffer->current_size = 0;

    tpl_resurrect_energy.power_previous_harvesting->index = 0;
    tpl_resurrect_energy.variance_buffer->index = 0;
    /* We set wake up to one, to avoid doing a prediction for the next step */
    tpl_resurrect_energy.wake_up = TRUE;
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
    #ifndef BARD
    P1OUT |= BIT2;
    #endif
    VAR(uint8_t, AUTOMATIC) i;
    CONSTP2CONST(tpl_step_ref, AUTOMATIC, OS_VAR)
    ptr_state = tpl_step_state[tpl_kern_resurrect.state];
    P2VAR(tpl_step, AUTOMATIC, OS_VAR) ptr_step = NULL;
    P2VAR(tpl_step, AUTOMATIC, OS_VAR) tmp_ptr_step = NULL;

    /* We compute energy consumed if using TIMER_ACTIVITY */
    #if WITH_TIMER_ACTIVITY
    uint8_t k;
    /* Voltage is i µV */
    int32_t voltage_harvested = 0;
    int32_t voltage_worst_case = ((int32_t)tpl_kern_resurrect.energy_at_start)*1000;
    int32_t voltage_consumed = ((int32_t)tpl_kern_resurrect.energy_at_start)*1000;
    /* Power is in µW */
    float power_harvested = 0;
    /* Time is in ms */
    uint32_t time_step = 0;
    /* Slope are in µV per ms and time is in ms*/
    // We could check online that wcet is always greater that measured time and if not, update it
    if(tpl_kern_resurrect.elected != NULL){
        for(k=0; k<tpl_kern_resurrect.elected->activity->nb_activity; k++){
            voltage_worst_case -= ((int32_t)tpl_kern_resurrect.elected->activity->slope[k] * (int32_t)tpl_kern_resurrect.elected->activity->time_activity[k]);
            voltage_consumed -= ((int32_t)tpl_kern_resurrect.elected->activity->slope[k] * (int32_t)tpl_kern_resurrect.elected->activity->current_time_activity[k]);
            time_step += (uint32_t)tpl_kern_resurrect.elected->activity->current_time_activity[k];
        }
    }
    #endif /* WITH_TIMER_ACTIVITY */

    P2VAR(uint16_t, AUTOMATIC, OS_VAR) result_adc_adc = &result_adc;
    uint16_t voltageInMillis;
    while (ptr_step == NULL)
    {
      /* Get energy level from ADC */
      bool use1V2Ref = true;
      tpl_adc_init_simple(use1V2Ref, result_adc_adc);
      /* Polling */
      result_adc = readPowerVoltage_simple();

      // uint16_t energy = (~result_adc)+1;
      uint16_t energy = result_adc;
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
      if((tpl_kern_resurrect.elected != NULL) & (tpl_resurrect_energy.wake_up == FALSE)){
          voltage_harvested = (((int32_t)voltageInMillis)*1000 - voltage_worst_case) - (voltage_consumed - voltage_worst_case);
          if(voltage_harvested < 0) {
              voltage_harvested = 0;
              power_harvested = 0;
          }
          else{
            float voltage_harvested_squared = (float) voltage_harvested * (float) voltage_harvested;
            /* Power is in µW, Capacitance in kF, Time in ms, Voltage in µV */
            power_harvested = ((voltage_harvested_squared * 0.5 * 0.0000068)) / ((float) time_step);
          }
          #ifdef debug_bet
          // #define FLOAT_TO_INT(x) ((x)>=0?(int16_t)((x)+0.5):(int16_t)((x)-0.5))
          #endif
      }
      #endif /* WITH_TIMER_ACTIVITY */
      #if WITH_ENERGY_PREDICTION
      if((tpl_kern_resurrect.elected != NULL) & (tpl_resurrect_energy.wake_up == FALSE)){
        const uint8_t index_power = (tpl_resurrect_energy.power_previous_harvesting->index++) % SMA_COUNT;

        tpl_resurrect_energy.power_previous_harvesting->buffer[index_power] = (uint32_t) power_harvested;

        if(tpl_resurrect_energy.power_previous_harvesting->current_size != SMA_COUNT){
            tpl_resurrect_energy.power_previous_harvesting->current_size++;
        }

        if(tpl_resurrect_energy.power_previous_harvesting->index == SMA_COUNT){
            tpl_resurrect_energy.power_previous_harvesting->index = 0;
        }

        if(tpl_resurrect_energy.power_prediction != 0){
            /* Error between previous prediction and current measure */
            tpl_resurrect_energy.error = (int32_t)tpl_resurrect_energy.power_prediction - (int32_t) power_harvested;
            #if WITH_BET
            /* We store error to buffer for variance */
            const uint8_t index_variance = (tpl_resurrect_energy.variance_buffer->index++) % 5;
            int32_t error_time = tpl_resurrect_energy.error * time_step;
            float error_time_float = (float) error_time;
            if(error_time_float < 0){
                error_time_float = -error_time_float;
            }
            float error_time_capacitance = (2 * error_time_float) / 6800000.0;
            /* We have error_v as V^2 -> to q12 for division and sqrt */
            _q12 error_v = _Q12sqrt(_Q12(error_time_capacitance));

            tpl_resurrect_energy.variance_buffer->buffer[index_variance] = error_v;
            if(tpl_resurrect_energy.variance_buffer->current_size != 5){
                tpl_resurrect_energy.variance_buffer->current_size++;
            }
            if(tpl_resurrect_energy.variance_buffer->index == 5){
                tpl_resurrect_energy.variance_buffer->index = 0;
            }
            /* We then compute variance */
            tpl_resurrect_energy.variance = tpl_variance_power_sma();
            #endif /* WITH_BET */
        }
        /* Next prediction */
        tpl_resurrect_energy.power_prediction = tpl_power_prediction_sma();
      }

      #endif /* WITH_ENERGY_PREDICTION */
      for (i = 0; i < ENERGY_LEVEL_COUNT; i++)
      {
        tmp_ptr_step = (P2VAR(tpl_step, AUTOMATIC, OS_VAR))ptr_state[i];
        if(tmp_ptr_step == NULL) continue;
        #if WITH_ENERGY_PREDICTION & WITH_BET == 1
        if ((tpl_kern_resurrect.elected == NULL) | (tpl_resurrect_energy.wake_up == TRUE)) {
        #else
        if (tpl_kern_resurrect.elected == NULL) {
        #endif /* WITH_ENERGY_PREDICTION - WITH_BET */
            if ((voltageInMillis >= tmp_ptr_step->energy) & (tmp_ptr_step != NULL)){
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
        else{
            if(tmp_ptr_step != NULL){
                #if WITH_ENERGY_PREDICTION & WITH_BET == 0
                if (voltageInMillis + (uint16_t)(tpl_resurrect_energy.prediction >> 3) >= tmp_ptr_step->energy)
                #elif WITH_ENERGY_PREDICTION & WITH_BET == 1
                float proba_threshold;
                if(tpl_kern_resurrect.award == 0) tpl_kern_resurrect.award = tmp_ptr_step->award;
                proba_threshold = 1.0 - ((float)tmp_ptr_step->award / (float) tpl_kern_resurrect.award);

                /* Proba_threshold should at least be 0.5 (we don't take more than 50% risk of power failure */
                if(proba_threshold < 0.75){
                    proba_threshold = 0.75;
                }
                /* If above voltage level, no need to compute probability of failing */
                if (voltageInMillis >= tmp_ptr_step->energy){
                    tpl_resurrect_energy.proba_power = 1.0;
                }
                else{
                    /* With power, we add every worstcase time to obtain time of the step */
                    uint32_t time_tmp_step = 0;
                    for(i=0; i<tmp_ptr_step->activity->nb_activity; i++){
                        time_tmp_step += tmp_ptr_step->activity->time_activity[i];
                    }
                    /* Delta_v is in nanoVolt */
                    float delta_v = ((float) tmp_ptr_step->delta_v / 1000000000.0);
                    _q12 delta_v_q12 = _Q12(delta_v);
                    /* Power prediction is in µW, time is in ms, we then have nJ */
                    float prediction_from_power = (float) ((float)tpl_resurrect_energy.power_prediction * (float)time_tmp_step);
                    /* We want to have V from power prediction -> V = sqrt(2*P*T/C) */
                    /* We have nJ, so with nF as capacitance, we have V */
                    float prediction_v2 = (2 * prediction_from_power) / 6800000.0;
                    /* We have prediction_v as V^2 -> to q12 for division and sqrt */
                    _q12 prediction_v = _Q12sqrt(_Q12(prediction_v2));
                    /* Current voltage is in milliVolt, scale to V */
                    float voltageInMillis_float = ((float)voltageInMillis / 1000.0);
                    _q12 voltageInMillis_q12 = _Q12(voltageInMillis_float);
                    _q12 mu = voltageInMillis_q12 - delta_v_q12 + prediction_v;

                    /* Ensure at least 0.1V of variance (usefull ?) */
                    if(tpl_resurrect_energy.variance < 410) tpl_resurrect_energy.variance = 410;

                    _q12 gaussian_q12 = gaussian(mu, tpl_resurrect_energy.variance, _Q12(1.9));

                    tpl_resurrect_energy.proba_power = 1.0 - _Q12toF(gaussian_q12);
                    if (tpl_resurrect_energy.proba_power < 1.0){
                        #ifndef BARD
                        P1OUT ^= BIT4;
                        #endif
                    }
                }
                if (tpl_resurrect_energy.proba_power > proba_threshold)
                #else
                if (voltageInMillis >= tmp_ptr_step->energy)
                #endif /* WITH_ENERGY_PREDICTION - WITH_BET */
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
            else{
                ptr_step = tmp_ptr_step;
            }
        }
      }

      /* Not enough energy to elect next step --> hibernate */
      #if WITH_BET
      /* If accumulated award reaches a threshold, we chkpt --> ptr_step == NULL --> break while loop */
      if((ptr_step == NULL) | (tpl_kern_resurrect.award >= THESHOLD_AWARD)){
          if(tpl_kern_resurrect.award >= THESHOLD_AWARD){
              #ifndef BARD
              P1OUT ^= BIT5;
              #endif
          }
      #else
      if (ptr_step == NULL){
      #endif /* WITH_BET */
        P8OUT |= BIT0;
        tpl_chkpt_hibernate();
        P8OUT &= ~BIT0;
        #if WITH_BET == YES

        if(tpl_kern_resurrect.schedule_from_hibernate == TRUE){
            /* Reset flag */
            tpl_kern_resurrect.schedule_from_hibernate = FALSE;
            break;
        }
        /* Might have sleep for some time, reset previous harvesting data */
        // tpl_resurrect_energy.previous_harvesting->current_size = 0;
        // tpl_resurrect_energy.previous_harvesting->index = 0;

        // tpl_resurrect_energy.power_previous_harvesting->current_size = 0;
        // tpl_resurrect_energy.power_previous_harvesting->index = 0;

        // tpl_resurrect_energy.variance_buffer->index = 0;
        // tpl_resurrect_energy.variance_buffer->current_size = 0;

        #endif /* WITH_ENERGY_PREDICTION - WITH_BET */
      }
    }
    #if WITH_TIMER_ACTIVITY
    /* Save Energy at start if starting a step */
    tpl_kern_resurrect.energy_at_start = voltageInMillis;
    #endif /* WITH_TIMER_ACTIVITY */
    #if WITH_ENERGY_PREDICTION & WITH_BET == 1
    tpl_resurrect_energy.wake_up = FALSE;
    #endif /* WITH_ENERGY_PREDICTION - WITH_BET */
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
    #if WITH_BET
    /* Update accumulated award */
    tpl_kern_resurrect.award += tpl_kern_resurrect.elected->award;
    #endif
    #ifndef BARD
    P1OUT &= ~BIT2;
    #endif
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

FUNC(uint32_t, OS_CODE) tpl_power_prediction_sma(void)
{
    uint8_t i;
    uint32_t result = 0;
    uint32_t tmp_result = 0;
    for(i=0; i<tpl_resurrect_energy.power_previous_harvesting->current_size; i++){
        tmp_result += tpl_resurrect_energy.power_previous_harvesting->buffer[i];
    }
    result = tmp_result / tpl_resurrect_energy.power_previous_harvesting->current_size;
    if (result == 0){
        return 1;
    }
    else{
        return result;
    }
}

#endif // ENERGY_PREDICTOR == "SMA"
#endif // WITH_ENERGY_PREDICTION

#if WITH_BET

FUNC(uint16_t, OS_CODE) tpl_variance_power_sma(void){
    uint8_t i;
    _q12 result;
    _q12 tmp = 0;
    for(i=0; i<tpl_resurrect_energy.variance_buffer->current_size; i++){
        tmp += tpl_resurrect_energy.variance_buffer->buffer[i];
    }
    result = _Q12div(tmp, _Q12((float)tpl_resurrect_energy.variance_buffer->current_size));
    return result;
}
#endif /* WITH_BET */
#endif // WITH_RESURRECT
