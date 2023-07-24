#include "msp430.h"
#include "tpl_os.h"

extern unsigned _chkptStartAddress_0;
extern unsigned _chkptStartAddress_1;
extern unsigned __data_start;
extern unsigned __tpl_end_of_checkpointing_zone;

#if WITH_CHECKPOINTING == YES
#define OS_START_SEC_CODE
#include "tpl_memmap.h"

FUNC(void, OS_CODE) tpl_save_checkpoint(sint16 tpl_checkpoint_buffer){
    if(tpl_checkpoint_buffer == -1){
        return;
    }
    unsigned *ptr_sram_start = & __data_start;
    unsigned *ptr_sram_stop = & __tpl_end_of_checkpointing_zone;
    
    if(tpl_checkpoint_buffer == 0){
    unsigned *ptr_chkpt = & _chkptStartAddress_0;
        while(ptr_sram_start != ptr_sram_stop){
            *ptr_chkpt = *ptr_sram_start;
            ptr_chkpt++;
            ptr_sram_start++;
        }
    }
    else{
        unsigned *ptr_chkpt = & _chkptStartAddress_1;
        while(ptr_sram_start != ptr_sram_stop){
            *ptr_chkpt = *ptr_sram_start;
            ptr_chkpt++;
            ptr_sram_start++;
        }
    }
    return;
}

FUNC(void, OS_CODE) tpl_load_checkpoint (sint16 tpl_checkpoint_buffer){
    if(tpl_checkpoint_buffer == -1){
        return;
    }
    unsigned *ptr_sram_start = & __data_start;
    unsigned *ptr_sram_stop = & __tpl_end_of_checkpointing_zone; 

    if(tpl_checkpoint_buffer == 0){
        unsigned *ptr_chkpt = & _chkptStartAddress_0;
        while(ptr_sram_start != ptr_sram_stop){
            *ptr_sram_start = *ptr_chkpt;
            ptr_chkpt++;
            ptr_sram_start++;
        }
    }
    else{
        unsigned *ptr_chkpt = & _chkptStartAddress_1;
        while(ptr_sram_start != ptr_sram_stop){
            *ptr_sram_start = *ptr_chkpt;
            ptr_chkpt++;
            ptr_sram_start++;
        }
    }
    return;  
}


// FUNC(void, OS_CODE) tpl_save_checkpoint_dma (){
//     // uintptr_t *ptr_chkpt = & _chkptStartAddress;
//     // uintptr_t *ptr_sram_start = & __data_start;
//     // uintptr_t *ptr_sram_stop = & __tpl_end_of_checkpointing_zone;

//     // __data20_write_long(&DMA0SA, &__data_start);
//     // __data20_write_long(&DMA0DA, &_chkptStartAddress);
//     // __data16_write_addr((unsigned short)&DMA0DA, &_chkptStartAddress);
//     // DMA0SA = (unsigned int)&__data_start;
//     // DMA0SZ = &__tpl_end_of_checkpointing_zone - &__data_start;
//     // DMA0CTL = DMADT_1 | DMASRCINCR_3 | DMADSTINCR_3 | DMASWDW | DMAEN | DMAREQ;

//     return;
// }

// FUNC(void, OS_CODE) tpl_load_checkpoint_dma (){
//     // uintptr_t *ptr_chkpt = & _chkptStartAddress;
//     // uintptr_t *ptr_sram_start = & __data_start;
//     // uintptr_t *ptr_sram_stop = & __tpl_end_of_checkpointing_zone; 
    
//     // __data20_write_long(&DMA0SA, &_chkptStartAddress);
//     __data16_write_addr( &DMA0SA, &_chkptStartAddress);
//     // __data20_write_long(&DMA0DA, &__data_start);
//     DMA0DA = (unsigned int)&__data_start;
//     DMA0SZ = &__tpl_end_of_checkpointing_zone - &__data_start;
//     DMA0CTL = DMADT_1 | DMASRCINCR_3 | DMADSTINCR_3 | DMASWDW | DMAEN | DMAREQ;
//     return;
// }
#define OS_STOP_SEC_CODE
#include "tpl_memmap.h"
#endif //WITH_CHECKPOINTING

