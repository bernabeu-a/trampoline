#include "tpl_os.h"
#include <stdint.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"

#include <math.h>

#include "tensorflow/lite/core/c/common.h"
#include "hello_world_float_model_data.h"
#include "hello_world_int8_model_data.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_profiler.h"
#include "tensorflow/lite/micro/recording_micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#define APP_Task_mnist_START_SEC_CODE
#include "tpl_memmap.h"

FUNC(int, OS_APPL_CODE) main(void){
	CHIP_Init();
	// Enable Clock to GPIO
	CMU_ClockEnable(cmuClock_GPIO, true);
	// Set GPIO Red Led to Output
	GPIO->P[gpioPortC].MODEL |= 4 << (16);
	StartOS(OSDEFAULTAPPMODE);
	return 0;
}

namespace {
	using HelloWorldOpResolver = tflite::MicroMutableOpResolver<1>;
	
	TfLiteStatus RegisterOps(HelloWorldOpResolver& op_resolver) {
	  TF_LITE_ENSURE_STATUS(op_resolver.AddFullyConnected());
	  return kTfLiteOk;
	}
}  // namespace

TfLiteStatus ProfileMemoryAndLatency() {
	tflite::MicroProfiler profiler;
	HelloWorldOpResolver op_resolver;
	TF_LITE_ENSURE_STATUS(RegisterOps(op_resolver));
	// // Arena size just a round number. The exact arena usage can be determined
	// // using the RecordingMicroInterpreter.
	constexpr int kTensorArenaSize = 3000;
	uint8_t tensor_arena[kTensorArenaSize];
	constexpr int kNumResourceVariables = 24;
  
	tflite::RecordingMicroAllocator* allocator(
		tflite::RecordingMicroAllocator::Create(tensor_arena, kTensorArenaSize));
	tflite::RecordingMicroInterpreter interpreter(
		tflite::GetModel(g_hello_world_float_model_data), op_resolver, allocator,
		tflite::MicroResourceVariables::Create(allocator, kNumResourceVariables),
		&profiler);
  
	TF_LITE_ENSURE_STATUS(interpreter.AllocateTensors());
	TFLITE_CHECK_EQ(interpreter.inputs_size(), 1);
	interpreter.input(0)->data.f[0] = 1.f;
	TF_LITE_ENSURE_STATUS(interpreter.Invoke());
  
	// profiler.LogTicksPerTagCsv();
  
	// interpreter.GetMicroAllocator().PrintAllocations();
	return kTfLiteOk;
  }

  TfLiteStatus LoadFloatModelAndPerformInference() {
	const tflite::Model* model = tflite::GetModel(g_hello_world_float_model_data);
	// TFLITE_CHECK_EQ(model->version(), TFLITE_SCHEMA_VERSION);
  
	HelloWorldOpResolver op_resolver;
	TF_LITE_ENSURE_STATUS(RegisterOps(op_resolver));
  
	// Arena size just a round number. The exact arena usage can be determined
	// using the RecordingMicroInterpreter.
	constexpr int kTensorArenaSize = 3000;
	uint8_t tensor_arena[kTensorArenaSize];
  
	tflite::MicroInterpreter interpreter(model, op_resolver, tensor_arena,
										 kTensorArenaSize);
	TF_LITE_ENSURE_STATUS(interpreter.AllocateTensors());
  
	// Check if the predicted output is within a small range of the
	// expected output
	float epsilon = 0.05f;
	constexpr int kNumTestValues = 4;
	float golden_inputs[kNumTestValues] = {0.f, 1.f, 3.f, 5.f};
  
	for (int i = 0; i < kNumTestValues; ++i) {
	  interpreter.input(0)->data.f[0] = golden_inputs[i];
	  TF_LITE_ENSURE_STATUS(interpreter.Invoke());
	  float y_pred = interpreter.output(0)->data.f[0];
	  TFLITE_CHECK_LE(abs(sin(golden_inputs[i]) - y_pred), epsilon);
	}
  
	return kTfLiteOk;
  }

  TfLiteStatus LoadQuantModelAndPerformInference() {
	// Map the model into a usable data structure. This doesn't involve any
	// copying or parsing, it's a very lightweight operation.
	const tflite::Model* model =
		::tflite::GetModel(g_hello_world_int8_model_data);
	TFLITE_CHECK_EQ(model->version(), TFLITE_SCHEMA_VERSION);
  
	HelloWorldOpResolver op_resolver;
	TF_LITE_ENSURE_STATUS(RegisterOps(op_resolver));
  
	// Arena size just a round number. The exact arena usage can be determined
	// using the RecordingMicroInterpreter.
	constexpr int kTensorArenaSize = 3000;
	uint8_t tensor_arena[kTensorArenaSize];
  
	tflite::MicroInterpreter interpreter(model, op_resolver, tensor_arena,
										 kTensorArenaSize);
  
	TF_LITE_ENSURE_STATUS(interpreter.AllocateTensors());
  
	TfLiteTensor* input = interpreter.input(0);
	TFLITE_CHECK_NE(input, nullptr);
  
	TfLiteTensor* output = interpreter.output(0);
	TFLITE_CHECK_NE(output, nullptr);
  
	float output_scale = output->params.scale;
	int output_zero_point = output->params.zero_point;
  
	// Check if the predicted output is within a small range of the
	// expected output
	float epsilon = 0.05;
  
	constexpr int kNumTestValues = 4;
	float golden_inputs_float[kNumTestValues] = {0.77, 1.57, 2.3, 3.14};
  
	// The int8 values are calculated using the following formula
	// (golden_inputs_float[i] / input->params.scale + input->params.zero_point)
	int8_t golden_inputs_int8[kNumTestValues] = {-96, -63, -34, 0};
  
	for (int i = 0; i < kNumTestValues; ++i) {
	  input->data.int8[0] = golden_inputs_int8[i];
	  TF_LITE_ENSURE_STATUS(interpreter.Invoke());
	  float y_pred = (output->data.int8[0] - output_zero_point) * output_scale;
	  TFLITE_CHECK_LE(abs(sin(golden_inputs_float[i]) - y_pred), epsilon);
	}
  
	return kTfLiteOk;
  }

TASK(mnist){
	// tflite::InitializeTarget();
	// ProfileMemoryAndLatency();
	LoadFloatModelAndPerformInference();
	LoadQuantModelAndPerformInference();

	GPIO->P[gpioPortC].DOUT ^= 1<<4;


	ChainTask(mnist);
}
#define APP_Task_mnist_STOP_SEC_CODE
#include "tpl_memmap.h"
