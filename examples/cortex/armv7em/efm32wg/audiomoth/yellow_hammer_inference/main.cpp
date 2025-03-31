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

	static tflite::MicroMutableOpResolver<>;
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
	GPIO->P[gpioPortC].DOUT ^= 1<<4;
	ChainTask(inference);
}
#define APP_Task_inference_STOP_SEC_CODE
#include "tpl_memmap.h"
