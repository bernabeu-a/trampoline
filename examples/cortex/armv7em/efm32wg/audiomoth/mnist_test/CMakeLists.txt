#to compile the project:
# mkdir _build
# cd _build
# cmake -D CMAKE_TOOLCHAIN_FILE=../main/compiler.cmake ..
# make
# make flash
cmake_minimum_required(VERSION 3.5)

# with this option, cmake will not try to compile a 
# simple test program (that may fail for an embedded target)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

#project name
project(main_exe)
enable_language(C CXX ASM)


set(TRAMPOLINE_BASE_PATH ${CMAKE_SOURCE_DIR}/../../../../../..)
set(TRAMPOLINE_MACHINE_PATH ${TRAMPOLINE_BASE_PATH}/machines)
set(TRAMPOLINE_GOIL_TEMPLATES ${TRAMPOLINE_BASE_PATH}/goil/templates)
set(APP_GENERATED_PATH ${CMAKE_SOURCE_DIR}/main)

#goil
set(OILFILE ${CMAKE_SOURCE_DIR}/main.oil)
set(OILCOMPILER goil)
set(OILFLAGS -t=cortex/armv7em/efm32wg --templates=${TRAMPOLINE_GOIL_TEMPLATES})

#cross compiler
include("main/compiler.cmake")

set(CFLAGS #CFLAGS
	#C flags from target options
	--specs=nosys.specs
	#C flags from .oil file
	-Og -g -DEFM32WG380F256=1
)

set(PRECFLAGS #PRE CFLAGS
	#PRE C flags from target options
)

set(CXXFLAGS #C++ FLAGS
	#C++ flags from target options
	-fno-rtti
	-felide-constructors
	-fno-threadsafe-statics
	-fno-use-cxa-get-exception-ptr
	-fno-enforce-eh-specs
	#C flags from .oil file
	-Og -g -DEFM32WG380F256=1
	-std=c++17
	-DTF_LITE_STATIC_MEMORY -DTF_LITE_DISABLE_X86_NEON -DNDEBUG
	-DCMSIS_NN -DKERNELS_OPTIMIZED_FOR_SPEED
	-DTF_LITE_MCU_DEBUG_LOG -DTF_LITE_STRIP_ERROR_STRINGS
	-mfpu=auto
	-Wnon-virtual-dtor
	-fno-unwind-tables
	-fno-asynchronous-unwind-tables
	-Wsign-compare -Wdouble-promotion -Wunused-variable -Wunused-function -Wswitch -Wvla -Wall -Wextra -Wmissing-field-initializers -Wstrict-aliasing -Wno-unused-parameter
	-mlittle-endian -Wno-type-limits -Wno-unused-private-field -fomit-frame-pointer
	-I/Users/bernabeu-a/bin/tflite-micro
	-I/Users/bernabeu-a/bin/tflite-micro/tensorflow/lite/micro/tools/make/downloads/flatbuffers/include
	-I/Users/bernabeu-a/bin/tflite-micro/tensorflow/lite/micro/tools/make/downloads/gemmlowp
	-I/Users/bernabeu-a/bin/tflite-micro/tensorflow/lite
	-I/Users/bernabeu-a/bin/tflite-micro/tensorflow/lite/c
	-I/Users/bernabeu-a/bin/tflite-micro/tensorflow/lite/core
	-I/Users/bernabeu-a/bin/tflite-micro/tensorflow/lite/core/api
	-I/Users/bernabeu-a/bin/tflite-micro/tensorflow/lite/core/c
	-I/Users/bernabeu-a/bin/tflite-micro/tensorflow/lite/kernels
	-I/Users/bernabeu-a/bin/tflite-micro/tensorflow/lite/kernels/internal
	-I/Users/bernabeu-a/bin/tflite-micro/tensorflow/lite/kernels/internal/reference
	-I/Users/bernabeu-a/bin/tflite-micro/tensorflow/compiler/mlir/lite/kernels/internal
	-I/Users/bernabeu-a/bin/tflite-micro/tensorflow/compiler/mlir/lite/core/api
	-I/Users/bernabeu-a/bin/tflite-micro/tensorflow/compiler/mlir/lite/core/c
	-I/Users/bernabeu-a/bin/tflite-micro/tensorflow/compiler/mlir/lite/schema
	-I/Users/bernabeu-a/bin/tflite-micro/tensorflow/lite/micro
	-I/Users/bernabeu-a/bin/tflite-micro/tensorflow/lite/micro/kernels
	-I/Users/bernabeu-a/bin/tflite-micro/tensorflow/lite/micro/arena_allocator
	-I/Users/bernabeu-a/bin/tflite-micro/tensorflow/lite/micro/memory_planner
	-I/Users/bernabeu-a/bin/tflite-micro/tensorflow/lite/micro/tflite_bridge
	-I/Users/bernabeu-a/bin/tflite-micro/tensorflow/lite/micro/tools/make
	-I/Users/bernabeu-a/bin/tflite-micro/tensorflow/lite/micro/make/downloads
	-I/Users/bernabeu-a/bin/tflite-micro/tensorflow/lite/micro/tools/make/downloads/kissfft
	-I/Users/bernabeu-a/bin/tflite-micro/tensorflow/lite/micro/tools/make/downloads/ruy/
	-I/Users/bernabeu-a/bin/tflite-micro/signal/src/kiss_fft_wrappers/
	-I/Users/bernabeu-a/bin/tflite-micro/signal/micro/kernels
	-I/Users/bernabeu-a/bin/tflite-micro/signal/src
)

set(PRECXXFLAGS #PRE C++ FLAGS
	#PRE C++ flags from target options
)

set(COMMONFLAGS #COMMON FLAGS (C/C++/ASM)
	#common flags from target options
	-DEFM32WG380F256=1
	-DARM_MATH_CM4=1
	-D__FPU_PRESENT=1
	-g
	-Wno-unused-but-set-variable
	-Wmissing-field-initializers
	-mcpu=cortex-m4
	-mthumb
	-mfloat-abi=hard
	-nostartfiles
	-fno-builtin
	-fno-exceptions
	-nostdlib
	-ffunction-sections
	-fdata-sections
)

set(PRECOMMONFLAGS #PRE COMMON FLAGS (C/C++/ASM)
	#PRE common flags from target options
)

add_compile_options(
  ${PRECOMMONFLAGS}
  ${COMMONFLAGS}
  "$<$<COMPILE_LANGUAGE:C>:${PRECFLAGS}>"
  "$<$<COMPILE_LANGUAGE:C>:${CFLAGS}>"
  "$<$<COMPILE_LANGUAGE:CXX>:${PRECXXFLAGS}>"
  "$<$<COMPILE_LANGUAGE:CXX>:${CXXFLAGS}>"
  "$<$<COMPILE_LANGUAGE:ASM>:${PRECFLAGS}>"
  "$<$<COMPILE_LANGUAGE:ASM>:${CFLAGS}>"
)

#Application sources
set(APP_SRCS
	#--- C++ files of the application
	main.cpp
	hello_world_float_model_data.cc
	hello_world_int8_model_data.cc
)

#Trampoline kernel sources 
set(TRAMPOLINE_OS_SRCS
	${TRAMPOLINE_BASE_PATH}/os/tpl_os_kernel.c
	${TRAMPOLINE_BASE_PATH}/os/tpl_os_timeobj_kernel.c
	${TRAMPOLINE_BASE_PATH}/os/tpl_os_action.c
	${TRAMPOLINE_BASE_PATH}/os/tpl_os_error.c
	${TRAMPOLINE_BASE_PATH}/os/tpl_os_os_kernel.c
	${TRAMPOLINE_BASE_PATH}/os/tpl_os_os.c
	${TRAMPOLINE_BASE_PATH}/os/tpl_os_interrupt_kernel.c
	${TRAMPOLINE_BASE_PATH}/os/tpl_os_task_kernel.c
	${TRAMPOLINE_BASE_PATH}/os/tpl_os_resource_kernel.c
) 

#Trampoline generated files
set(APP_GENERATED_SRCS
	#os generated files
	${APP_GENERATED_PATH}/tpl_app_config.c
	${APP_GENERATED_PATH}/tpl_dispatch_table.c
	${APP_GENERATED_PATH}/tpl_invoque.S
	#platform dependant generated files
	${APP_GENERATED_PATH}/tpl_primary_irq.S
	${APP_GENERATED_PATH}/tpl_vectors.c
	${APP_GENERATED_PATH}/tpl_external_interrupts.c
	${APP_GENERATED_PATH}/tpl_app_interrupts.c
)

#Trampoline target dependant files
set(TRAMPOLINE_MACHINE_SRCS
	${TRAMPOLINE_MACHINE_PATH}/cortex/tpl_machine_cortex.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/tpl_sc_handler.S
	${TRAMPOLINE_MACHINE_PATH}/cortex/tpl_startup.S
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/tpl_ctx_switch.S
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/tpl_ctx_switch_under_it.S
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/tpl_interrupts.S
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/startup_efm32wg.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/system_efm32wg.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/handlers_efm32wg.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/tpl_machine_efm32wg.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_acmp.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_adc.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_aes.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_assert.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_burtc.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_can.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_cmu.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_core.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_cryotimer.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_crypto.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_csen.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_dac.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_dbg.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_dma.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_ebi.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_emu.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_gpcrc.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_gpio.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_i2c.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_idac.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_lcd.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_ldma.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_lesense.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_letimer.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_leuart.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_mpu.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_msc.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_opamp.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_pcnt.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_prs.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_qspi.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_rmu.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_rtc.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_rtcc.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_system.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_timer.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_usart.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_vcmp.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_vdac.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src/em_wdog.c
)

# Trampoline target library sources (drivers)
set(TARGET_LIBRARY_SRCS
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/MatrixFunctions/MatrixFunctionsF16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/MatrixFunctions/MatrixFunctions.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/TransformFunctions/TransformFunctions.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/TransformFunctions/TransformFunctionsF16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/CommonTables/CommonTablesF16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/CommonTables/CommonTables.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/FilteringFunctions/FilteringFunctionsF16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/FilteringFunctions/FilteringFunctions.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/StatisticsFunctions/StatisticsFunctionsF16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/StatisticsFunctions/StatisticsFunctions.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/BasicMathFunctions/BasicMathFunctions.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/BasicMathFunctions/BasicMathFunctionsF16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/InterpolationFunctions/InterpolationFunctions.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/InterpolationFunctions/InterpolationFunctionsF16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/SupportFunctions/SupportFunctionsF16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/SupportFunctions/SupportFunctions.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/DistanceFunctions/DistanceFunctions.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/DistanceFunctions/DistanceFunctionsF16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/ControllerFunctions/ControllerFunctions.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/FastMathFunctions/FastMathFunctions.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/FastMathFunctions/FastMathFunctionsF16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/WindowFunctions/WindowFunctions.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/BayesFunctions/BayesFunctionsF16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/BayesFunctions/BayesFunctions.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/SVMFunctions/SVMFunctions.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/SVMFunctions/SVMFunctionsF16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/QuaternionMathFunctions/QuaternionMathFunctions.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/ComplexMathFunctions/ComplexMathFunctions.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Source/ComplexMathFunctions/ComplexMathFunctionsF16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/PoolingFunctions/arm_avgpool_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/PoolingFunctions/arm_avgpool_get_buffer_sizes_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/PoolingFunctions/arm_max_pool_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/PoolingFunctions/arm_avgpool_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/PoolingFunctions/arm_avgpool_get_buffer_sizes_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/PoolingFunctions/arm_max_pool_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_mat_mult_nt_interleaved_t_even_s4.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_transpose_conv_row_s8_s32.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_depthwise_conv_nt_t_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_vec_mat_mult_t_svdf_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_mat_mul_core_1x_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_lstm_step_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_vec_mat_mult_t_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_depthwise_conv_nt_t_padded_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_lstm_calculate_gate_s8_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_mat_mult_nt_t_s8_s32.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nntables.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_mat_mul_core_4x_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_mat_mult_nt_t_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_vec_mat_mult_t_per_ch_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_lstm_calculate_gate_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_vec_mat_mul_result_acc_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_depthwise_conv_nt_t_s4.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_lstm_step_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_vec_mat_mult_t_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_mat_mul_core_1x_s4.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_mat_mult_nt_t_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_depthwise_conv_nt_t_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_vec_mat_mul_result_acc_s8_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_q7_to_q15_with_offset.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_vec_mat_mult_t_s4.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_s8_to_s16_unordered_with_offset.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_mat_mult_nt_t_s4.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/NNSupportFunctions/arm_nn_vec_mat_mult_t_s16_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/PadFunctions/arm_pad_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/FullyConnectedFunctions/arm_fully_connected_get_buffer_sizes_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/FullyConnectedFunctions/arm_fully_connected_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/FullyConnectedFunctions/arm_fully_connected_per_channel_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/FullyConnectedFunctions/arm_fully_connected_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/FullyConnectedFunctions/arm_fully_connected_get_buffer_sizes_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/FullyConnectedFunctions/arm_vector_sum_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/FullyConnectedFunctions/arm_batch_matmul_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/FullyConnectedFunctions/arm_batch_matmul_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/FullyConnectedFunctions/arm_fully_connected_wrapper_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/FullyConnectedFunctions/arm_fully_connected_s4.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/FullyConnectedFunctions/arm_vector_sum_s8_s64.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConcatenationFunctions/arm_concatenation_s8_x.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConcatenationFunctions/arm_concatenation_s8_w.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConcatenationFunctions/arm_concatenation_s8_y.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConcatenationFunctions/arm_concatenation_s8_z.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/SoftmaxFunctions/arm_softmax_u8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/SoftmaxFunctions/arm_nn_softmax_common_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/SoftmaxFunctions/arm_softmax_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/SoftmaxFunctions/arm_softmax_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/SoftmaxFunctions/arm_softmax_s8_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/SVDFunctions/arm_svdf_get_buffer_sizes_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/SVDFunctions/arm_svdf_state_s16_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/SVDFunctions/arm_svdf_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ActivationFunctions/arm_relu_q15.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ActivationFunctions/arm_nn_activation_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ActivationFunctions/arm_relu6_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ActivationFunctions/arm_relu_q7.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/BasicMathFunctions/arm_elementwise_mul_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/BasicMathFunctions/arm_elementwise_add_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/BasicMathFunctions/arm_elementwise_mul_s16_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/BasicMathFunctions/arm_elementwise_mul_acc_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/BasicMathFunctions/arm_elementwise_mul_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/BasicMathFunctions/arm_maximum_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/BasicMathFunctions/arm_elementwise_mul_s16_batch_offset.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/BasicMathFunctions/arm_elementwise_add_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/BasicMathFunctions/arm_minimum_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/LSTMFunctions/arm_lstm_unidirectional_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/LSTMFunctions/arm_lstm_unidirectional_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_depthwise_conv_get_buffer_sizes_s4.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_nn_mat_mult_kernel_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_nn_mat_mult_kernel_s4_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_depthwise_conv_wrapper_s4.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_depthwise_conv_get_buffer_sizes_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_depthwise_conv_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_transpose_conv_get_buffer_sizes_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_convolve_1_x_n_s4.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_convolve_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_depthwise_conv_s4.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_depthwise_conv_s4_opt.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_convolve_1x1_s4_fast.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_convolve_1x1_s8_fast.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_convolve_s4.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_convolve_1x1_s4.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_convolve_wrapper_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_depthwise_conv_3x3_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_convolve_get_buffer_sizes_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_convolve_wrapper_s4.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_nn_mat_mult_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_depthwise_conv_wrapper_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_depthwise_conv_get_buffer_sizes_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_transpose_conv_wrapper_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_nn_depthwise_conv_s8_core.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_convolve_1_x_n_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_depthwise_conv_s8_opt.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_depthwise_conv_fast_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_transpose_conv_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_convolve_even_s4.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_convolve_1x1_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_convolve_get_buffer_sizes_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_convolve_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_depthwise_conv_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_nn_mat_mult_kernel_row_offset_s8_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_convolve_wrapper_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_depthwise_conv_wrapper_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_convolve_get_buffer_sizes_s4.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ConvolutionFunctions/arm_nn_mat_mult_kernel_s8_s16.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/TransposeFunctions/arm_transpose_s8.c
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Source/ReshapeFunctions/arm_reshape_s8.c
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/kernels/kernel_util.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/core/api/tensor_utils.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/core/api/flatbuffer_conversions.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/core/c/common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/compiler/mlir/lite/core/api/error_reporter.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/compiler/mlir/lite/schema/schema_utils.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/kernels/internal/tensor_utils.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/kernels/internal/common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/kernels/internal/portable_tensor_utils.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/kernels/internal/tensor_ctypes.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/kernels/internal/runtime_shape.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/kernels/internal/reference/portable_tensor_utils.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/kernels/internal/reference/comparisons.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/kernels/internal/quantization_util.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/debug_log.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/fake_micro_context.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/flatbuffer_utils.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/hexdump.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/memory_helpers.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/micro_allocation_info.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/micro_allocator.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/micro_context.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/micro_interpreter_context.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/micro_interpreter_graph.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/micro_interpreter.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/micro_log.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/micro_op_resolver.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/micro_profiler.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/micro_resource_variable.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/micro_time.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/micro_utils.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/mock_micro_graph.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/recording_micro_allocator.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/arena_allocator/non_persistent_arena_buffer_allocator.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/arena_allocator/persistent_arena_buffer_allocator.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/arena_allocator/recording_single_arena_buffer_allocator.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/arena_allocator/single_arena_buffer_allocator.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/memory_planner/greedy_memory_planner.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/memory_planner/linear_memory_planner.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/memory_planner/non_persistent_buffer_planner_shim.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/tflite_bridge/flatbuffer_conversions_bridge.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/tflite_bridge/micro_error_reporter.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/activations.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/activations_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/add_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/add_n.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/arg_min_max.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/assign_variable.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/batch_matmul_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/batch_to_space_nd.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/broadcast_args.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/broadcast_to.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/call_once.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/cast.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/ceil.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/circular_buffer.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/circular_buffer_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/comparisons.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/concatenation.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/conv_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/cumsum.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/decompress.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/decompress_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/depth_to_space.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/depthwise_conv_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/dequantize.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/dequantize_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/detection_postprocess.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/div.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/elementwise.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/elu.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/embedding_lookup.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/ethosu.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/exp.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/expand_dims.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/fill.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/floor.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/floor_div.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/floor_mod.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/fully_connected_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/gather.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/gather_nd.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/hard_swish.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/hard_swish_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/if.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/kernel_runner.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/kernel_util.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/l2norm.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/l2_pool_2d.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/leaky_relu.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/leaky_relu_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/logical.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/logical_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/logistic.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/logistic_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/log_softmax.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/lstm_eval.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/lstm_eval_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/micro_tensor_utils.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/mirror_pad.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/mul.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/mul_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/neg.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/pack.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/pad.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/pad_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/pooling_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/prelu.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/prelu_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/quantize.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/quantize_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/read_variable.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/reduce.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/reduce_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/reshape.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/reshape_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/resize_bilinear.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/resize_nearest_neighbor.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/round.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/select.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/shape.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/slice.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/softmax_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/space_to_batch_nd.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/space_to_depth.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/split.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/split_v.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/squared_difference.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/squeeze.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/strided_slice.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/strided_slice_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/sub.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/sub_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/svdf_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/tanh.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/transpose_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/unpack.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/var_handle.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/while.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/zeros_like.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/add.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/batch_matmul.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/conv.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/depthwise_conv.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/fully_connected.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/maximum_minimum.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/pooling.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/softmax.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/svdf.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/transpose.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/transpose_conv.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/unidirectional_sequence_lstm.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/micro/kernels/delay.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/micro/kernels/energy.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/micro/kernels/fft_auto_scale_kernel.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/micro/kernels/fft_auto_scale_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/micro/kernels/filter_bank.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/micro/kernels/filter_bank_log.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/micro/kernels/filter_bank_square_root.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/micro/kernels/filter_bank_square_root_common.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/micro/kernels/filter_bank_spectral_subtraction.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/micro/kernels/framer.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/micro/kernels/irfft.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/micro/kernels/rfft.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/micro/kernels/stacker.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/micro/kernels/overlap_add.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/micro/kernels/pcan.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/micro/kernels/window.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/circular_buffer.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/energy.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/fft_auto_scale.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/filter_bank.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/filter_bank_log.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/filter_bank_square_root.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/filter_bank_spectral_subtraction.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/irfft_float.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/irfft_int16.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/irfft_int32.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/log.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/max_abs.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/msb_32.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/msb_64.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/overlap_add.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/pcan_argc_fixed.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/rfft_float.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/rfft_int16.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/rfft_int32.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/square_root_32.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/square_root_64.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/window.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/kiss_fft_wrappers/kiss_fft_float.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/kiss_fft_wrappers/kiss_fft_int16.cc
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro/signal/src/kiss_fft_wrappers/kiss_fft_int32.cc
)

#Include directories
include_directories(
	${TRAMPOLINE_BASE_PATH}/com
	${TRAMPOLINE_BASE_PATH}/debug
	${TRAMPOLINE_BASE_PATH}/ioc
	${TRAMPOLINE_BASE_PATH}/os
	${TRAMPOLINE_MACHINE_PATH}/../../../bin/tflite-micro
	${TRAMPOLINE_MACHINE_PATH}/cortex
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/Include
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-DSP/PrivateInclude
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS-NN/Include
	${TRAMPOLINE_MACHINE_PATH}/cortex/CMSIS_5/CMSIS/Core/Include
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/inc
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/emlib/src
	${TRAMPOLINE_MACHINE_PATH}/cortex/armv7em/efm32wg/inc
	main
)

#Executable (should be defined after compile options)
add_executable(${PROJECT_NAME} 
    ${APP_SRCS}
    ${TRAMPOLINE_OS_SRCS}
    ${APP_GENERATED_SRCS}
	${TARGET_LIBRARY_SRCS}
    ${TRAMPOLINE_MACHINE_SRCS}
)

#Linker
set(LINKER_DIR ${APP_GENERATED_PATH})
set(LINKER_SCRIPT script.ld)
set(LINKER_FLAGS 
	#linker flags from target options
	--fatal-warnings
	--warn-common
	--no-undefined
	--gc-sections
	#linker flags from .oil file
	-Map=main.map
	-nostdlib
	-L/Applications/ARM/arm-none-eabi/lib/thumb/v7e-m+fp/hard
	#link script
	-L${LINKER_DIR}
 -T${LINKER_SCRIPT}
)

#
function(add_gcc_library libCmd)
  #use gcc to get the full path to libs (libgcc, libc)
  execute_process(
    COMMAND ${CMAKE_C_COMPILER} ${PRECOMMONFLAGS} ${PRECFLAGS} ${COMMONFLAGS} ${CFLAGS} ${libCmd}
    RESULT_VARIABLE returnValue
    OUTPUT_VARIABLE full_lib_path
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  if (${returnValue} EQUAL 0)
    #ok, now extract only the directory
    get_filename_component(libgcc_path ${full_lib_path} DIRECTORY)
    set(libpath ${libgcc_path} PARENT_SCOPE)
  else()
    message("library ${libCmd} not found!")
  endif()
endfunction()

if(${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
	add_gcc_library("-print-libgcc-file-name") #try to find libgcc
	list(APPEND LINKER_FLAGS -L${libpath} -lgcc)
	add_gcc_library("-print-file-name=libc.a") #try to find libc
	list(APPEND LINKER_FLAGS -L${libpath} -lc)
endif()

#should be defined after 'add_executable'
target_link_libraries(${PROJECT_NAME}
	${LINKER_FLAGS}
)

set_property(TARGET ${PROJECT_NAME} 
    PROPERTY LINK_DEPENDS ${LINKER_DIR}/${LINKER_SCRIPT}
)

#post build commands (bin file, flash, …)
# POSTBUILD rules
add_custom_command(OUTPUT ${PROJECT_NAME}.bin
    DEPENDS ${PROJECT_NAME}
    COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME} ${PROJECT_NAME}.bin
	WORKING_DIRECTORY ${CMAKE_BUILD_DIR}
)

#TODO: add alias avec dependance.
#add_custom_target(burn
#    DEPENDS flash
#)

#goil (will update generated files, and depends on .oil input file)
add_custom_command(OUTPUT ${APP_GENERATED_SRCS} 
    DEPENDS ${OILFILE}
    COMMAND ${OILCOMPILER} ${OILFLAGS} ${OILFILE}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} #build dir
    COMMENT "call Goil (.oil to code source)"
)

