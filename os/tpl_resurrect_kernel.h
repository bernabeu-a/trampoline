/**
 * @file tpl_sequence_kernel.h
 *
 * @section descr File description
 *
 * Trampoline sequence os services header.
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

#ifndef TPL_OS_RESURRECT_H
#define TPL_OS_RESURRECT_H

#include "tpl_compiler.h"
#include "tpl_os_internal_types.h"
#include "tpl_os_kernel.h"
#include "tpl_os_alarm_kernel.h"
#include <stdint.h>
#if WITH_BET == YES
#include "QmathLib.h"
#endif
/**
 * @typedef EventResurrectType
 *
 * identifies an event for resurrect
 *
 */
typedef uint16_t tpl_event_resurrect;

typedef tpl_event_resurrect  EventResurrectType;

typedef int NumberActivationType;

struct TPL_ACTIVITY
{
  CONST(uint8_t, TYPEDEF) nb_activity;
  P2CONST(uint16_t, TYPEDEF, OS_VAR) slope;
  P2CONST(uint16_t, TYPEDEF, OS_VAR) time_activity;
  P2VAR(uint16_t, TYPEDEF, OS_VAR) current_time_activity;
};

typedef struct TPL_ACTIVITY tpl_activity;

struct TPL_STEP
{
  CONST(uint32, TYPEDEF) energy;
  CONST(uint8, TYPEDEF) to_state;
  CONST(uint8, TYPEDEF) from_state;
  CONST(tpl_proc_function, TYPEDEF) entry_point;
  #if WITH_RESURRECT_EVENT == YES
  CONST(EventResurrectType, TYPEDEF) resurrect_event[RESURRECT_EVENT_COUNT];
  #endif /* WITH_RESURRECT_EVENT */
  #if WITH_TIMER_ACTIVITY == YES
  CONSTP2CONST(tpl_activity,TYPEDEF, OS_VAR) activity;
  #endif /* WITH_TIMER_ACTIVITY */
  #if WITH_BET
  CONST(uint32, TYPEDEF) delta_v;
  CONST(uint16, TYPEDEF) award;
  #endif /* WITH_BET */
};

typedef struct TPL_STEP tpl_step;
typedef tpl_step const *tpl_step_ref;

#if WITH_BET
struct TPL_VARIANCE_BUFFER
{
    _q12 buffer[5];
    uint8_t index;
    uint8_t current_size;
};
typedef struct TPL_VARIANCE_BUFFER tpl_variance_buffer;
#endif /* WITH_BET */

typedef struct
{
  P2VAR(tpl_step, TYPEDEF, OS_VAR) elected;
  VAR(sint32, TYPEDEF) state;
  #if WITH_RESURRECT_EVENT == YES
  VAR(uint16_t, TYPEDEF) state_event;
  #endif /* WITH_RESURRECT_EVENT */
  #if WITH_TIMER_ACTIVITY
  VAR(uint16_t, TYPEDEF) energy_at_start;
  #endif /* WITH_TIMER_ACTIVITY */
  #if WITH_BET
  VAR(uint16_t, TYPEDEF) award;
  #endif /* WITH_BET */
} tpl_kern_resurrect_state;

#if WITH_ENERGY_PREDICTION == YES
#if ENERGY_PREDICTOR == SMA
struct TPL_ENERGY_BUFFER
{
    uint32_t buffer[SMA_COUNT];
    uint8_t index;
    uint8_t current_size;
};
typedef struct TPL_ENERGY_BUFFER tpl_energy_buffer;
#endif // ENERGY_PREDICTOR == SMA
struct TPL_ENERGY
{
    P2VAR(tpl_energy_buffer, TYPEDEF, OS_VAR) power_previous_harvesting;
    VAR(uint32_t, TYPEDEF) power_prediction;
    VAR(int32_t, TYPEDEF) error;
    #if WITH_BET
    VAR(float, TYPEDEF) proba_power;
    VAR(uint8_t, TYPEDEF) wake_up;
    VAR(uint16_t, TYPEDEF) variance;
    P2VAR(tpl_variance_buffer, TYPEDEF, OS_VAR) variance_buffer;
    VAR(uint16_t, TYPEDEF) v_before_sleeping;
    VAR(uint8_t, TYPEDEF) hibernate_second;
    VAR(uint8_t, TYPEDEF) hibernate_minute;
    #endif /* WITH_BET */
};
typedef struct TPL_ENERGY tpl_energy;
#endif // WITH_ENERGY_PREDICTION

#define OS_START_SEC_VAR_UNSPECIFIED
#include "tpl_memmap.h"

#if WITH_ENERGY_PREDICTION == YES
extern VAR(tpl_energy, OS_VAR) tpl_resurrect_energy;
#endif // WITH_ENERGY_PREDICTION
extern VAR(tpl_kern_resurrect_state, OS_VAR) tpl_kern_resurrect;

#define OS_STOP_SEC_VAR_UNSPECIFIED
#include "tpl_memmap.h"

#define OS_START_SEC_CONST_UNSPECIFIED
#include "tpl_memmap.h"

extern CONSTP2CONST(tpl_step_ref, AUTOMATIC,
                    OS_APPL_DATA) tpl_step_state[RESURRECT_STATE_COUNT];

#define OS_STOP_SEC_CONST_UNSPECIFIED
#include "tpl_memmap.h"

#define OS_START_SEC_VAR_NON_VOLATILE_16BIT
#include "tpl_memmap.h"
extern VAR(uint16_t, AUTOMATIC) result_adc;
#define OS_STOP_SEC_VAR_NON_VOLATILE_16BIT
#include "tpl_memmap.h"

#define OS_START_SEC_CODE
#include "tpl_memmap.h"

FUNC(void, OS_CODE)
tpl_start_os_resurrect_service(CONST(tpl_application_mode, AUTOMATIC) mode);

FUNC(void, OS_CODE) tpl_terminate_step_resurrect_service(void);

FUNC(void, OS_CODE) tpl_choose_next_step(void);

FUNC(void, OS_CODE) tpl_set_activation_alarm_service(CONST(tpl_alarm_id, AUTOMATIC)  alarm_id, VAR(int, AUTOMATIC) set_nb_activation);

#if WITH_ENERGY_PREDICTION == YES
#if ENERGY_PREDICTOR == SMA
// FUNC(uint32_t, OS_CODE) tpl_prediction_sma(void);
FUNC(uint32_t, OS_CODE) tpl_power_prediction_sma(void);
#endif // ENERGY_PREDICTOR == "SMA"
#if WITH_BET
// FUNC(uint32_t, OS_CODE) tpl_variance_sma(void);
FUNC(uint16_t, OS_CODE) tpl_variance_power_sma(void);
#endif // WITH_BET
#endif // WITH_ENERGY_PREDICTOR
#define OS_STOP_SEC_CODE
#include "tpl_memmap.h"

/* TPL_OS_RESURRECT_H */
#endif
/* End of file tpl_resurrect_kernel.h */
