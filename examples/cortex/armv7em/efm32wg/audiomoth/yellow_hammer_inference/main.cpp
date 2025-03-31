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


namespace {
	const tflite::Model* model = nullptr;
	tflite::MicroInterpreter* interpreter = nullptr;
	TfLiteTensor* input = nullptr;

	// Are of memory to use for input, output and intermediate array
	// Size is max output layer ?
	constexpr int kTensorArenaSize = 136*1024;
	alignas(16) static uint8_t tensor_arena[kTensorArenaSize];
}	//namespace

void setup_model(){
	model = tflite::GetModel(model_data);
	// Check some stuff ? version etc ?

	static tflite::MicroMutableOpResolver<23> micro_op_resolver;
	micro_op_resolver.AddExpandDims();
	micro_op_resolver.AddConv2D();
	micro_op_resolver.AddMul();
	micro_op_resolver.AddAdd();
	micro_op_resolver.AddReshape();
	micro_op_resolver.AddExpandDims();
	micro_op_resolver.AddMaxPool2D();
	micro_op_resolver.AddReshape();
	micro_op_resolver.AddExpandDims();
	micro_op_resolver.AddConv2D();
	micro_op_resolver.AddMul();
	micro_op_resolver.AddAdd();
	micro_op_resolver.AddReshape();
	micro_op_resolver.AddExpandDims();
	micro_op_resolver.AddMaxPool2D();
	micro_op_resolver.AddReshape();
	micro_op_resolver.AddExpandDims();
	micro_op_resolver.AddConv2D();
	micro_op_resolver.AddReshape();
	micro_op_resolver.AddMean();
	micro_op_resolver.AddFullyConnected();
	micro_op_resolver.AddFullyConnected();
	micro_op_resolver.AddLogistic();

	static tflite::MicroInterpreter static_interpreter(model, micro_op_resolver, tensor_arena, kTensorArenaSize);
	interpreter = &static_interpreter;
	TfLiteSatus allocate_status = interpreter->AllocateTensors();
	input = interpreter->input(0);
	return;
}

#define APP_Task_inference_START_SEC_CODE
#include "tpl_memmap.h"

FUNC(int, OS_APPL_CODE) main(void){
	CHIP_Init();
	// Enable Clock to GPIO
	CMU_ClockEnable(cmuClock_GPIO, true);
	// Set GPIO Red Led to Output
	GPIO->P[gpioPortC].MODEL |= 4 << (16);
	setup_model();
	StartOS(OSDEFAULTAPPMODE);
	return 0;
}

TASK(inference){
	// Get data for input
	input->data.int8 = envelop_data;
	interpreter->Invoke();
	TfLiteTensor* output = interpreter->output(0);
	uint8_t final_output = output->data.uint8_t[0];
	GPIO->P[gpioPortC].DOUT ^= 1<<4;
	ChainTask(inference);
}
#define APP_Task_inference_STOP_SEC_CODE
#include "tpl_memmap.h"
