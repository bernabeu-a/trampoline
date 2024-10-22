#include "tpl_machine.h"
#include "tpl_machine_interface.h"
#include "tpl_os_application_def.h"
#include "tpl_os_definitions.h"
#include "tpl_os.h"

#if WITH_AUTOSAR == YES
	#include "tpl_as_definitions.h"
#endif
#include "tpl_os_interrupt.h"
#include "tpl_kern_stack.h"

#include "msp430.h"



#define OS_START_SEC_CODE
#include "tpl_memmap.h"

FUNC(uint8, OS_CODE) tpl_lpm_isr2(void){
    GET_CURRENT_CORE_ID(core_id);
    if(TPL_KERN(core_id).running->exit_from_lpm == TRUE) return 1;
    else return 0;
}


FUNC(void, OS_CODE) set_wake_up_from_isr2(CONST(uint16, AUTOMATIC) isr_id){
    CONSTP2VAR(tpl_proc, AUTOMATIC, OS_VAR) isr = tpl_dyn_proc_table[isr_id];
    isr->exit_from_lpm = TRUE;
    return;
}

FUNC(void, OS_CODE) reset_wake_up_from_isr2(CONST(uint16, AUTOMATIC) isr_id){
    CONSTP2VAR(tpl_proc, AUTOMATIC, OS_VAR) isr = tpl_dyn_proc_table[isr_id];
    isr->exit_from_lpm = FALSE;
    return;
}
#define OS_STOP_SEC_CODE
#include "tpl_memmap.h"
