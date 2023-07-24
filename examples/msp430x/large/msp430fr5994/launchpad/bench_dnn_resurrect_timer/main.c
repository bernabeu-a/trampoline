/**
 * \file main.c
 *
 * Adapted from:
 * https://github.com/tinysystems/ImmortalThreads/tree/main/runtime/test/benchmarks/dnn/main.c
 */

#include "msp430.h"
#include "tpl_os.h"

#include "includes/dnnlib/dnn.h"
#include "includes/fixedlib/fixed.h"
#include "includes/matlib/mat.h"

#include "headers/conv1.h"
#include "headers/conv2.h"
#include "headers/fc1.h"
#include "headers/fc2.h"
#include "headers/input.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define APP_Task_resurrect_START_SEC_VAR_NON_VOLATILE_UNSPECIFIED
#include "tpl_memmap.h"
VAR(mat_t, AUTOMATIC) mat_conv1_wd = {
    .dims = {CONV1_WMD_LEN},
    .len_dims = 1,
    .strides = {1},
    .data = conv1_wmd,
    .sparse =
        {
            .dims = {20, 1, 1, 1},
            .len_dims = 4,
            .sizes = conv1_wmd_sizes,
            .offsets = conv1_wmd_offsets,
        },
};

VAR(mat_t, AUTOMATIC) mat_conv1_wv = {
    .dims = {CONV1_WMV_LEN},
    .len_dims = 1,
    .strides = {1},
    .data = conv1_wmv,
    .sparse =
        {
            .dims = {20, 1, 5, 1},
            .len_dims = 4,
            .sizes = conv1_wmv_sizes,
            .offsets = conv1_wmv_offsets,
        },
};

VAR(mat_t, AUTOMATIC) mat_conv1_wh = {
    .dims = {CONV1_WMH_LEN},
    .len_dims = 1,
    .strides = {1},
    .data = conv1_wmh,
    .sparse =
        {
            .dims = {20, 1, 1, 5},
            .len_dims = 4,
            .sizes = conv1_wmh_sizes,
            .offsets = conv1_wmh_offsets,
        },
};

VAR(mat_t, AUTOMATIC) mat_conv1_b = {
    .dims = {20},
    .len_dims = 1,
    .strides = {1},
    .data = conv1_b,
};

VAR(mat_t, AUTOMATIC) mat_conv2_w = {
    .dims = {CONV2_WM_LEN},
    .len_dims = 1,
    .strides = {1},
    .data = conv2_wm,
    .sparse =
        {
            .dims = {100, 20, 5, 5},
            .len_dims = 4,
            .sizes = conv2_wm_sizes,
            .offsets = conv2_wm_offsets,
        },
};

VAR(mat_t, AUTOMATIC) mat_conv2_b = {
    .dims = {100},
    .strides = {1},
    .len_dims = 1,
    .data = conv2_b,
};

VAR(mat_t, AUTOMATIC) mat_fc1_wh = {
    .dims = {FC1_WMH_LEN},
    .len_dims = 1,
    .strides = {1},
    .data = fc1_wmh,
    .sparse =
        {
            .dims = {100, 1600},
            .len_dims = 2,
            .offsets = fc1_wmh_offsets,
            .sizes = fc1_wmh_sizes,
        },
};

VAR(mat_t, AUTOMATIC) mat_fc1_wv = {
    .dims = {FC1_WMV_LEN},
    .len_dims = 1,
    .strides = {1},
    .data = fc1_wmv,
    .sparse =
        {
            .dims = {500, 100},
            .len_dims = 2,
            .offsets = fc1_wmv_offsets,
            .sizes = fc1_wmv_sizes,
        },
};

VAR(mat_t, AUTOMATIC) mat_fc1_b = {
    .dims = {500, 1},
    .strides = {1, 1},
    .len_dims = 2,
    .data = fc1_b,
};

VAR(mat_t, AUTOMATIC) mat_fc2_w = {
    .dims = {10, 500},
    .strides = {500, 1},
    .len_dims = 2,
    .data = fc2_w,
};

VAR(mat_t, AUTOMATIC) mat_fc2_b = {
    .dims = {10, 1},
    .strides = {1, 1},
    .len_dims = 2,
    .data = fc2_b,
};

VAR(mat_t, AUTOMATIC) mat_input = {
    .dims = {1, 28, 28},
    .strides = {784, 28, 1},
    .len_dims = 3,
    .data = input,
};

VAR(mat_t, AUTOMATIC) buf1 = {.data = layer_buffers[0]};
VAR(mat_t, AUTOMATIC) buf2 = {.data = layer_buffers[1]};
VAR(mat_t*, AUTOMATIC) b1 = &buf1;
VAR(mat_t*, AUTOMATIC) b2 = &buf2;
#define APP_Task_resurrect_STOP_SEC_VAR_NON_VOLATILE_UNSPECIFIED
#include "tpl_memmap.h"

#define APP_COMMON_START_SEC_CODE
#include "tpl_memmap.h"

FUNC(int, OS_APPL_CODE) main(void){
    P1DIR |= BIT0 + BIT4 + BIT5;
    P1OUT &= ~(BIT0 + BIT4 + BIT5);
    // P3DIR = 0xFF;
    // tpl_serial_begin(SERIAL_TX_MODE_BLOCK);

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;
    // StartOS(OSDEFAULTAPPMODE);
    // __bic_SR_register(GIE);
	StartResurrect(OSDEFAULTAPPMODE);
    return 0;
}

FUNC(int, OS_APPL_CODE) restart_main(void){
    P1DIR |= BIT0 + BIT4 + BIT5;
    P1OUT &= ~(BIT0 + BIT4 + BIT5);
    // P3DIR = 0xFF;
    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;
    P1OUT |= BIT5;
    RestartOS();
    return 0;
}


void dnnmain(void){
    P1OUT |= BIT0;
    params.same_padding = false;
    params.size[0] = 1;
    params.size[1] = 2;
    params.size[2] = 2;
    params.stride[0] = 1;
    params.stride[1] = 1;
    params.stride[2] = 1;

    MAT_RESHAPE(b2, 1, 28, 28);
    mat_t *mat_input_ptr = &mat_input;
    for (uint16_t i = 0; i < 28; i++) {
        for (uint16_t j = 0; j < 28; j++) {
            fixed w = MAT_GET(mat_input_ptr, 0, i, j);
            MAT_SET(b2, w, 0, i, j);
        }
    }
    P1OUT &= ~BIT0;
    return;
}

void convol1D (void) {
    MAT_RESHAPE(b1, 20, 28, 28);
    mat_t *w_ptr = &mat_conv1_wd;
    mat_t *b_ptr = NULL;
    convol(b2, b1, w_ptr, b_ptr);
    return;
}

void convol1H (void){
    mat_t *w_ptr = &mat_conv1_wh;
    mat_t *b_ptr = NULL;
    MAT_RESHAPE(b2, 20, 28, 24);
    convol_depth(b1, b2, w_ptr, b_ptr);
    return;
}

void convol1W (void) {
    mat_t *b_ptr = &mat_conv1_b;
    mat_t *w_ptr = &mat_conv1_wv;
    MAT_RESHAPE(b1, 20, 24, 24);
    convol_depth(b2, b1, w_ptr, b_ptr);
    return;
}

void relu1 (void){
    MAT_RESHAPE(b2, 20, 24, 24);
    relu(b1, b2);
    return;
}

void pooling1 (void) {
    MAT_RESHAPE(b1, 20, 12, 12);
    params.stride[1] = 2;
    params.stride[2] = 2;
    pooling(b2, b1);
    return;
}

void convol2 (void) {
    MAT_RESHAPE(b2, 100, 8, 8);
    params.stride[1] = 1;
    params.stride[2] = 1;
    mat_t *b_ptr = &mat_conv2_b;
    mat_t *w_ptr = &mat_conv2_w;
    convol(b1, b2, w_ptr, b_ptr);
    return;
}

void relu2 (void) {
    MAT_RESHAPE(b1, 100, 8, 8);
    relu(b2, b1);
    return;
}

void pooling2 (void) {
    MAT_RESHAPE(b2, 100, 4, 4);
    params.stride[1] = 2;
    params.stride[2] = 2;
    pooling(b1, b2);
    return;
}

void fc1h (void) {
    MAT_RESHAPE(b2, 1600, 1);
    MAT_RESHAPE(b1, 100, 1);
    mat_t *w_ptr = &mat_fc1_wh;
    mat_t *b_ptr = NULL;
    fc_sparse(b2, b1, w_ptr, b_ptr);
    return;
}

void fc1w (void) {
    MAT_RESHAPE(b2, 500, 1);
    mat_t *w_ptr = &mat_fc1_wv;
    mat_t *b_ptr = &mat_fc1_b;
    fc_sparse(b1, b2, w_ptr, b_ptr);
    return;
}

void relu3 (void) {
    MAT_RESHAPE(b1, 500, 1);
    relu(b2, b1);
    return;
}

void fc2 (void) {
    MAT_RESHAPE(b2, 10, 1);
    mat_t *w_ptr = &mat_fc2_w;
    mat_t *b_ptr = &mat_fc2_b;
    fc_dense(b1, b2, w_ptr, b_ptr);
    return;
}

void prediction (void){
    uint16_t predict = pred(b2);
    if (predict == 3) {
        P1OUT &= ~BIT0;
	}
    return;
}
#define APP_COMMON_STOP_SEC_CODE
#include "tpl_memmap.h"

#define APP_ISR_adc_sampling_START_SEC_CODE
#include "tpl_memmap.h"

ISR(adc_sampling)
{

}

#define APP_ISR_adc_sampling_STOP_SEC_CODE
#include "tpl_memmap.h"
