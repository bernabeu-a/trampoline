set(TENSORFLOW_ROOT /Users/bernabeu-a/bin/tflite-micro/)
set(CMAKE_VERBOSE_MAKEFILE on)

include_directories(
    ${TENSORFLOW_ROOT}tensorflow/lite
    ${TENSORFLOW_ROOT}tensorflow/lite/c
    ${TENSORFLOW_ROOT}tensorflow/lite/core
    ${TENSORFLOW_ROOT}tensorflow/lite/core/api
    ${TENSORFLOW_ROOT}tensorflow/lite/core/c
    ${TENSORFLOW_ROOT}tensorflow/lite/kernels
    ${TENSORFLOW_ROOT}tensorflow/lite/kernels/internal
    ${TENSORFLOW_ROOT}tensorflow/lite/kernels/internal/reference
    ${TENSORFLOW_ROOT}tensorflow/compiler/mlir/lite/kernels/internal
    ${TENSORFLOW_ROOT}tensorflow/compiler/mlir/lite/core/api
    ${TENSORFLOW_ROOT}tensorflow/compiler/mlir/lite/core/c
    ${TENSORFLOW_ROOT}tensorflow/compiler/mlir/lite/schema
    ${TENSORFLOW_ROOT}tensorflow/lite/micro
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/arena_allocator
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/memory_planner
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/tflite_bridge
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/tools/make
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/tools/make/downloads
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/tools/make/downloads/gemmlowp
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/tools/make/downloads/flatbuffers/include
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/tools/make/downloads/kissfft
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/tools/make/downloads/ruy/
    ${TENSORFLOW_ROOT}signal/src/kiss_fft_wrappers/
    ${TENSORFLOW_ROOT}signal/micro/kernels
    ${TENSORFLOW_ROOT}signal/src

)

set(TFLITE_SRCS
    # LITE_KERNELS_SRCS_CC
    ${TENSORFLOW_ROOT}tensorflow/lite/kernels/kernel_util.cc
    # LITE_CORE_API_SRCS_CC
    ${TENSORFLOW_ROOT}tensorflow/lite/core/api/tensor_utils.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/core/api/flatbuffer_conversions.cc
    # LITE_CORE_C_SRCS_CC
    ${TENSORFLOW_ROOT}tensorflow/lite/core/c/common.cc
    # COMPILER_MLIR_LITE_CORE_API_SRCS_CC
    ${TENSORFLOW_ROOT}tensorflow/compiler/mlir/lite/core/api/error_reporter.cc
    # COMPILER_MLIR_LITE_SCHEMA_SRCS_CC
    ${TENSORFLOW_ROOT}tensorflow/compiler/mlir/lite/schema/schema_utils.cc
    # LITE_KERNELS_INTERNAL_CC_SRCS
    ${TENSORFLOW_ROOT}tensorflow/lite/kernels/internal/tensor_utils.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/kernels/internal/common.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/kernels/internal/portable_tensor_utils.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/kernels/internal/tensor_ctypes.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/kernels/internal/runtime_shape.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/kernels/internal/reference/portable_tensor_utils.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/kernels/internal/reference/comparisons.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/kernels/internal/quantization_util.cc
    # MICROLITE_CC_SRCS
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/debug_log.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/fake_micro_context.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/flatbuffer_utils.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/hexdump.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/memory_helpers.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/micro_allocation_info.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/micro_allocator.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/micro_context.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/micro_interpreter_context.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/micro_interpreter_graph.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/micro_interpreter.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/micro_log.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/micro_op_resolver.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/micro_profiler.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/micro_resource_variable.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/micro_time.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/micro_utils.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/mock_micro_graph.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/recording_micro_allocator.cc
    # MICROLITE_CC_ARENA_ALLOCATOR_SRCS
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/arena_allocator/non_persistent_arena_buffer_allocator.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/arena_allocator/persistent_arena_buffer_allocator.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/arena_allocator/recording_single_arena_buffer_allocator.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/arena_allocator/single_arena_buffer_allocator.cc
    # MICROLITE_CC_MEMORY_PLANNER_SRCS
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/memory_planner/greedy_memory_planner.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/memory_planner/linear_memory_planner.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/memory_planner/non_persistent_buffer_planner_shim.cc
    # MICROLITE_CC_TFLITE_BRIDGE_SRCS
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/tflite_bridge/flatbuffer_conversions_bridge.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/tflite_bridge/micro_error_reporter.cc
    # MICROLITE_CC_KERNEL_SRCS
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/activations.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/activations_common.cc
    # ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/add.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/add_common.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/add_n.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/arg_min_max.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/assign_variable.cc 
    # ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/batch_matmul.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/batch_matmul_common.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/batch_to_space_nd.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/broadcast_args.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/broadcast_to.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/call_once.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/cast.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/ceil.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/circular_buffer.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/circular_buffer_common.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/comparisons.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/concatenation.cc 
    # ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/conv.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/conv_common.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/cumsum.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/decompress.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/decompress_common.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/depth_to_space.cc 
    # ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/depthwise_conv.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/depthwise_conv_common.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/dequantize.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/dequantize_common.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/detection_postprocess.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/div.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/elementwise.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/elu.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/embedding_lookup.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/ethosu.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/exp.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/expand_dims.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/fill.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/floor.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/floor_div.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/floor_mod.cc 
    # ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/fully_connected.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/fully_connected_common.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/gather.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/gather_nd.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/hard_swish.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/hard_swish_common.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/if.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/kernel_runner.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/kernel_util.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/l2norm.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/l2_pool_2d.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/leaky_relu.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/leaky_relu_common.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/logical.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/logical_common.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/logistic.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/logistic_common.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/log_softmax.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/lstm_eval.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/lstm_eval_common.cc 
    # ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/maximum_minimum.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/micro_tensor_utils.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/mirror_pad.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/mul.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/mul_common.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/neg.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/pack.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/pad.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/pad_common.cc 
    # ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/pooling.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/pooling_common.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/prelu.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/prelu_common.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/quantize.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/quantize_common.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/read_variable.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/reduce.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/reduce_common.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/reshape.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/reshape_common.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/resize_bilinear.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/resize_nearest_neighbor.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/round.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/select.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/shape.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/slice.cc 
    # ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/softmax.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/softmax_common.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/space_to_batch_nd.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/space_to_depth.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/split.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/split_v.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/squared_difference.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/squeeze.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/strided_slice.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/strided_slice_common.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/sub.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/sub_common.cc 
    # ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/svdf.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/svdf_common.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/tanh.cc 
    # ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/transpose.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/transpose_common.cc 
    # ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/transpose_conv.cc 
    # ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/unidirectional_sequence_lstm.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/unpack.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/var_handle.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/while.cc 
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/zeros_like.cc    
    # Optimized kernel because we use cmsisnn
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/cmsis_nn/add.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/cmsis_nn/batch_matmul.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/cmsis_nn/conv.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/cmsis_nn/depthwise_conv.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/cmsis_nn/fully_connected.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/cmsis_nn/maximum_minimum.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/cmsis_nn/pooling.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/cmsis_nn/softmax.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/cmsis_nn/svdf.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/cmsis_nn/transpose.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/cmsis_nn/transpose_conv.cc
    ${TENSORFLOW_ROOT}tensorflow/lite/micro/kernels/cmsis_nn/unidirectional_sequence_lstm.cc
    # MICROLITE_CC_SIGNAL_KERNEL_SRCS
    ${TENSORFLOW_ROOT}signal/micro/kernels/delay.cc 
    ${TENSORFLOW_ROOT}signal/micro/kernels/energy.cc 
    ${TENSORFLOW_ROOT}signal/micro/kernels/fft_auto_scale_kernel.cc 
    ${TENSORFLOW_ROOT}signal/micro/kernels/fft_auto_scale_common.cc 
    ${TENSORFLOW_ROOT}signal/micro/kernels/filter_bank.cc 
    ${TENSORFLOW_ROOT}signal/micro/kernels/filter_bank_log.cc 
    ${TENSORFLOW_ROOT}signal/micro/kernels/filter_bank_square_root.cc 
    ${TENSORFLOW_ROOT}signal/micro/kernels/filter_bank_square_root_common.cc 
    ${TENSORFLOW_ROOT}signal/micro/kernels/filter_bank_spectral_subtraction.cc 
    ${TENSORFLOW_ROOT}signal/micro/kernels/framer.cc 
    ${TENSORFLOW_ROOT}signal/micro/kernels/irfft.cc 
    ${TENSORFLOW_ROOT}signal/micro/kernels/rfft.cc 
    ${TENSORFLOW_ROOT}signal/micro/kernels/stacker.cc 
    ${TENSORFLOW_ROOT}signal/micro/kernels/overlap_add.cc 
    ${TENSORFLOW_ROOT}signal/micro/kernels/pcan.cc 
    ${TENSORFLOW_ROOT}signal/micro/kernels/window.cc 
    ${TENSORFLOW_ROOT}signal/src/circular_buffer.cc 
    ${TENSORFLOW_ROOT}signal/src/energy.cc 
    ${TENSORFLOW_ROOT}signal/src/fft_auto_scale.cc 
    ${TENSORFLOW_ROOT}signal/src/filter_bank.cc 
    ${TENSORFLOW_ROOT}signal/src/filter_bank_log.cc 
    ${TENSORFLOW_ROOT}signal/src/filter_bank_square_root.cc 
    ${TENSORFLOW_ROOT}signal/src/filter_bank_spectral_subtraction.cc 
    ${TENSORFLOW_ROOT}signal/src/irfft_float.cc 
    ${TENSORFLOW_ROOT}signal/src/irfft_int16.cc 
    ${TENSORFLOW_ROOT}signal/src/irfft_int32.cc 
    ${TENSORFLOW_ROOT}signal/src/log.cc 
    ${TENSORFLOW_ROOT}signal/src/max_abs.cc 
    ${TENSORFLOW_ROOT}signal/src/msb_32.cc 
    ${TENSORFLOW_ROOT}signal/src/msb_64.cc 
    ${TENSORFLOW_ROOT}signal/src/overlap_add.cc 
    ${TENSORFLOW_ROOT}signal/src/pcan_argc_fixed.cc 
    ${TENSORFLOW_ROOT}signal/src/rfft_float.cc 
    ${TENSORFLOW_ROOT}signal/src/rfft_int16.cc 
    ${TENSORFLOW_ROOT}signal/src/rfft_int32.cc 
    ${TENSORFLOW_ROOT}signal/src/square_root_32.cc 
    ${TENSORFLOW_ROOT}signal/src/square_root_64.cc 
    ${TENSORFLOW_ROOT}signal/src/window.cc
    # Additionnal kernel 
    ${TENSORFLOW_ROOT}signal/src/kiss_fft_wrappers/kiss_fft_float.cc
    ${TENSORFLOW_ROOT}signal/src/kiss_fft_wrappers/kiss_fft_int16.cc
    ${TENSORFLOW_ROOT}signal/src/kiss_fft_wrappers/kiss_fft_int32.cc
)