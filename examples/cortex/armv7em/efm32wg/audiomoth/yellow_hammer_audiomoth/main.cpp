#include "tpl_os.h"
#include <stdint.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"

#include <math.h>

#include "tensorflow/lite/core/c/common.h"
#include "input/input.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_profiler.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

/* Debugging */
#include "tensorflow/lite/micro/recording_micro_interpreter.h"

#include "input/input.h"
#include "model/bands_int8_model_model_data.h"

namespace {
	using YellowHammerOpResolver = tflite::MicroMutableOpResolver<9>;

	TfLiteStatus RegisterOps(YellowHammerOpResolver& op_resolver){
		op_resolver.AddExpandDims();
		op_resolver.AddConv2D();
		op_resolver.AddMul();
		op_resolver.AddAdd();
		op_resolver.AddReshape();
		op_resolver.AddMaxPool2D();
		op_resolver.AddMean();
		op_resolver.AddFullyConnected();
		op_resolver.AddLogistic();
		return kTfLiteOk;
	}

	tflite::MicroInterpreter *interpreter = nullptr;
	TfLiteTensor *input = nullptr;
	TfLiteTensor *output = nullptr;
	const tflite::Model* model = nullptr;
	TfLiteStatus myStatus;
	// // Are of memory to use for input, output and intermediate array
	// // Size is max output layer ?
	// constexpr int kTensorArenaSize = 2720;
	// alignas(16) static uint8_t tensor_arena[kTensorArenaSize];

}	//namespace

TfLiteStatus AllocateTensor(){
	TF_LITE_ENSURE_STATUS(interpreter->AllocateTensors());
}

TfLiteStatus ProcessInference(){
	TF_LITE_ENSURE_STATUS(interpreter->Invoke());
}

#define APP_Task_setup_inference_START_SEC_CODE
#include "tpl_memmap.h"
TASK(setup_inference){
	/* Get Model */
	model = ::tflite::GetModel(bands_int8_model_tflite);
	if(model->version() != TFLITE_SCHEMA_VERSION){
		while(1);
	}
	/* Ops */
	YellowHammerOpResolver op_resolver;
	RegisterOps(op_resolver);
	/* ScratchPad Memory for Layers */
	constexpr int kTensorArenaSize = 11500;
	uint8_t tensor_arena[kTensorArenaSize];
	static tflite::MicroInterpreter static_interpreter(model, op_resolver, tensor_arena, kTensorArenaSize);
	interpreter = &static_interpreter;
	myStatus = AllocateTensor();
	if(myStatus != kTfLiteOk){
		while(1);
	}
	/* Input */
	input = interpreter->input(0);
	TFLITE_CHECK_NE(input, nullptr);
	/* Output */
	output = interpreter->output(0);
	TFLITE_CHECK_NE(output, nullptr);

	ChainTask(inference);
}
#define APP_Task_setup_inference_STOP_SEC_CODE
#include "tpl_memmap.h"

#define APP_Task_inference_START_SEC_CODE
#include "tpl_memmap.h"

FUNC(int, OS_APPL_CODE) main(void){
	CHIP_Init();
	// Enable Clock to GPIO
	CMU_ClockEnable(cmuClock_GPIO, true);
	// Set GPIO Red Led to Output
	GPIO->P[gpioPortC].MODEL |= 4 << (16);
	// setup_model();
	StartOS(OSDEFAULTAPPMODE);
	return 0;
}

TASK(inference){	// 500ms

	GPIO->P[gpioPortA].DOUT |= (1<<7);

	// Get data for input
	uint16_t i;

	// float input_scale = input->params.scale;
	float input_scale = 0.00392157;
    // int input_zero_point = input->params.zero_point;
	int input_zero_point = -128;
	float output_scale = 0.00390625;
	int output_zero_point = -128;


	int8_t quantized_value[261] = {0};
	for(i=0; i<261; i++){
		int value = static_cast<int>(round((envelop_data[i] / input_scale) + input_zero_point));
		value = std::min(std::max(value, -128), 127);
		quantized_value[i] = static_cast<int8_t>(value);
		input->data.int8[i] = quantized_value[i];
	}
	/* Now put quantized data on input of NN */
	// input->data.int8[0] = *quantized_value;
	/* Process NN */
	myStatus = ProcessInference();
	if(myStatus != kTfLiteOk){
		while(1);
	}
	// // int8_t final_output;
	// // std::copy_n(tflite::GetTensorData<int8_t>(output), 1, &final_output);
	int8_t final_output = output->data.int8[0];
	float output_score = static_cast<float>(static_cast<int>(final_output) - output_zero_point) * output_scale;

	if(output_score > 0.6f){
		// GPIO->P[gpioPortA].DOUT |= (1<<7);
		GPIO->P[gpioPortA].DOUT &= ~(1<<7);
	}
	GPIO->P[gpioPortA].DOUT &= ~(1<<7);
	ChainTask(inference);
}
#define APP_Task_inference_STOP_SEC_CODE
#include "tpl_memmap.h"
