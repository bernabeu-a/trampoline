/* trampoline stuff */
#include "tpl_os.h"

/* ----- TFLM header ----- */
#include "tensorflow/lite/core/c/common.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_profiler.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

/* ----- Model TFLM ----- */
// #include "model/bands_int8_model_model_data.h"
#include "model/cmsis_bands_max_low_int8_model.h"

#include "arm_math.h"

#include "fir_coeffs.h"
/* We only share envelopes between both src files */
// extern VAR(float, AUTOMATIC) envelope_1 [80];
// extern VAR(float, AUTOMATIC) envelope_2 [80];
// extern VAR(float, AUTOMATIC) envelope_3 [80];

/* Envelopes are in SRAM ext, we only share pointers */
extern P2VAR(float, AUTOMATIC, AUTOMATIC) ping_ptr_envelope1;
extern P2VAR(float, AUTOMATIC, AUTOMATIC) ping_ptr_envelope2;
extern P2VAR(float, AUTOMATIC, AUTOMATIC) ping_ptr_envelope3;

extern P2VAR(float, AUTOMATIC, AUTOMATIC) pong_ptr_envelope1;
extern P2VAR(float, AUTOMATIC, AUTOMATIC) pong_ptr_envelope2;
extern P2VAR(float, AUTOMATIC, AUTOMATIC) pong_ptr_envelope3;

extern VAR(bool, AUTOMATIC) envelop_ping_rdy;

extern VAR(uint8_t, AUTOMATIC) sram_ping_rdy;

extern P2VAR(float32_t, AUTOMATIC, AUTOMATIC) ptr_SRAM_FILTERED_AUDIO_PING_1;
extern P2VAR(float32_t, AUTOMATIC, AUTOMATIC) ptr_SRAM_FILTERED_AUDIO_PING_2;
extern P2VAR(float32_t, AUTOMATIC, AUTOMATIC) ptr_SRAM_FILTERED_AUDIO_PING_3;

extern P2VAR(float32_t, AUTOMATIC, AUTOMATIC) ptr_SRAM_FILTERED_AUDIO_PONG_1;
extern P2VAR(float32_t, AUTOMATIC, AUTOMATIC) ptr_SRAM_FILTERED_AUDIO_PONG_2;
extern P2VAR(float32_t, AUTOMATIC, AUTOMATIC) ptr_SRAM_FILTERED_AUDIO_PONG_3;


extern VAR(float32_t, AUTOMATIC) max_pool_for_low_pass1 [80];
extern P2VAR(uint32_t, AUTOMATIC, AUTOMATIC) indexMax1;
extern VAR(float32_t, AUTOMATIC) max_pool_for_low_pass2 [80];
extern P2VAR(uint32_t, AUTOMATIC, AUTOMATIC) indexMax2;
extern VAR(float32_t, AUTOMATIC) max_pool_for_low_pass3 [80];
extern P2VAR(uint32_t, AUTOMATIC, AUTOMATIC) indexMax3;

extern VAR(uint8_t, AUTOMATIC) index_buffer_audio_inference;
extern VAR(uint8_t, AUTOMATIC) index_buffer_audio_inference2;

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
	// model = ::tflite::GetModel(bands_int8_model_tflite);
	model = ::tflite::GetModel(cmsis_bands_max_low_int8_model_tflite);

	if(model->version() != TFLITE_SCHEMA_VERSION){
		while(1);
	}
	/* Ops */
	YellowHammerOpResolver op_resolver;
	RegisterOps(op_resolver);
	/* ScratchPad Memory for Layers */
	constexpr int kTensorArenaSize = 10000;
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

VAR(float, AUTOMATIC) input_inference[240];


static float32_t firStateF32_low1 [2*2] = {0};
static float32_t firStateF32_low2 [2*2] = {0};
static float32_t firStateF32_low3 [2*2] = {0};

TASK(inference){

	float *tmp_ptr_envelope1_inference;
	float *tmp_ptr_envelope2_inference;
	float *tmp_ptr_envelope3_inference;

	EventMaskType ev1;
	WaitEvent(ev_NORMALIZE);
	GetEvent(inference, &ev1);
	ClearEvent(ev1);
	GPIO->P[gpioPortA].DOUT |= 1<<8;

	index_buffer_audio_inference2 = index_buffer_audio_inference;

	// GPIO->P[gpioPortA].DOUT |= (1<<7);

#ifdef WITH_MAX_NOT_IN_INFERENCE

#else
	arm_biquad_cascade_df2T_instance_f32 instFilter_low1;
	arm_biquad_cascade_df2T_instance_f32 instFilter_low2;
	arm_biquad_cascade_df2T_instance_f32 instFilter_low3;

	arm_biquad_cascade_df2T_init_f32(&instFilter_low1, 2, &firCoefF32_low[0], &firStateF32_low1[0]);
	arm_biquad_cascade_df2T_init_f32(&instFilter_low2, 2, &firCoefF32_low[0], &firStateF32_low2[0]);
	arm_biquad_cascade_df2T_init_f32(&instFilter_low3, 2, &firCoefF32_low[0], &firStateF32_low3[0]);
	if(sram_ping_rdy){
		sram_ping_rdy = false;
		/* Max on 500ms of sound */
		for(uint8_t i=0; i<80; i++){
			arm_max_f32(ptr_SRAM_FILTERED_AUDIO_PING_1+(128*i), 128, max_pool_for_low_pass1+i, indexMax1);
			arm_max_f32(ptr_SRAM_FILTERED_AUDIO_PING_2+(128*i), 128, max_pool_for_low_pass2+i, indexMax2);
			arm_max_f32(ptr_SRAM_FILTERED_AUDIO_PING_3+(128*i), 128, max_pool_for_low_pass3+i, indexMax3);
		}
		
		// arm_biquad_cascade_df2T_instance_f32 instFilter_low1;
		// arm_biquad_cascade_df2T_instance_f32 instFilter_low2;
		// arm_biquad_cascade_df2T_instance_f32 instFilter_low3;

		arm_biquad_cascade_df2T_f32(&instFilter_low1, max_pool_for_low_pass1, ping_ptr_envelope1, 80);
		arm_biquad_cascade_df2T_f32(&instFilter_low2, max_pool_for_low_pass2, ping_ptr_envelope2, 80);
		arm_biquad_cascade_df2T_f32(&instFilter_low3, max_pool_for_low_pass3, ping_ptr_envelope3, 80);

		envelop_ping_rdy = 1;

	}
	else{
		sram_ping_rdy = true;
		/* Max on 500ms of sound */
		for(uint8_t i= 0; i<80; i++){
			arm_max_f32(ptr_SRAM_FILTERED_AUDIO_PONG_1+(128*i), 128, max_pool_for_low_pass1+i, indexMax1);
			arm_max_f32(ptr_SRAM_FILTERED_AUDIO_PONG_2+(128*i), 128, max_pool_for_low_pass2+i, indexMax2);
			arm_max_f32(ptr_SRAM_FILTERED_AUDIO_PONG_3+(128*i), 128, max_pool_for_low_pass3+i, indexMax3);
		}
		// arm_biquad_cascade_df2T_instance_f32 instFilter_low1;
		// arm_biquad_cascade_df2T_instance_f32 instFilter_low2;
		// arm_biquad_cascade_df2T_instance_f32 instFilter_low3;

		arm_biquad_cascade_df2T_f32(&instFilter_low1, max_pool_for_low_pass1, pong_ptr_envelope1, 80);
		arm_biquad_cascade_df2T_f32(&instFilter_low2, max_pool_for_low_pass2, pong_ptr_envelope2, 80);
		arm_biquad_cascade_df2T_f32(&instFilter_low3, max_pool_for_low_pass3, pong_ptr_envelope3, 80);

		envelop_ping_rdy = 0;
	}
	
#endif
	float min_band1 = 999.0;
	float max_band1 = 0;

	float min_band2 = 999.0;
	float max_band2 = 0;

	float min_band3 = 999.0;
	float max_band3 = 0;

	/* If we are here, it means we have 80 data in envelopes */
	/* We normalize for inference input */
	/* First get min/max */
	if(envelop_ping_rdy){
		tmp_ptr_envelope1_inference = ping_ptr_envelope1;
		tmp_ptr_envelope2_inference = ping_ptr_envelope2;
		tmp_ptr_envelope3_inference = ping_ptr_envelope3;
	}
	else{
		tmp_ptr_envelope1_inference = pong_ptr_envelope1;
		tmp_ptr_envelope2_inference = pong_ptr_envelope2;
		tmp_ptr_envelope3_inference = pong_ptr_envelope3;
	}

	for(uint8_t i = 0; i < 80; i++){
		if(*tmp_ptr_envelope1_inference < min_band1) {
			min_band1 = *tmp_ptr_envelope1_inference;
		}
		if(*tmp_ptr_envelope1_inference > max_band1) {
			max_band1 = *tmp_ptr_envelope1_inference;
		}
		tmp_ptr_envelope1_inference++;
		if(*tmp_ptr_envelope2_inference < min_band2) {
			min_band2 = *tmp_ptr_envelope2_inference;
		}
		if(*tmp_ptr_envelope2_inference > max_band2) {
			max_band2 = *tmp_ptr_envelope2_inference;
		}
		tmp_ptr_envelope2_inference++;
		if(*tmp_ptr_envelope3_inference < min_band3) {
			min_band3 = *tmp_ptr_envelope3_inference;
		}
		if(*tmp_ptr_envelope3_inference > max_band3) {
			max_band3 = *tmp_ptr_envelope3_inference;
		}
		tmp_ptr_envelope3_inference++;
		// *tmp_ptr_envelope1_inference++;
		// *tmp_ptr_envelope1_inference++;
		// *tmp_ptr_envelope1_inference++;
	}
	/* Then normalize */
	if(envelop_ping_rdy){
		tmp_ptr_envelope1_inference = ping_ptr_envelope1;
		tmp_ptr_envelope2_inference = ping_ptr_envelope2;
		tmp_ptr_envelope3_inference = ping_ptr_envelope3;
	}
	else{
		tmp_ptr_envelope1_inference = pong_ptr_envelope1;
		tmp_ptr_envelope2_inference = pong_ptr_envelope2;
		tmp_ptr_envelope3_inference = pong_ptr_envelope3;
	}
	for(uint8_t i = 0; i < 80; i++){
		input_inference[3*i] = (*tmp_ptr_envelope1_inference++ - min_band1) / (max_band1 - min_band1);
		input_inference[3*i+1] = (*tmp_ptr_envelope2_inference++ - min_band2) / (max_band2 - min_band2);
		input_inference[3*i+2] = (*tmp_ptr_envelope3_inference++ - min_band3) / (max_band3 - min_band3);
	}

	// Get data for input
	uint16_t i;

	// float input_scale = input->params.scale;
	const float input_scale = 0.00392157;
    // int input_zero_point = input->params.zero_point;
	const int input_zero_point = -128;
	const float output_scale = 0.00390625;
	const int output_zero_point = -128;


	int8_t quantized_value[240] = {0};
	for(i=0; i<240; i++){
		int value = static_cast<int>(round((input_inference[i] / input_scale) + input_zero_point));
		value = std::min(std::max(value, -128), 127);
		quantized_value[i] = static_cast<int8_t>(value);
		input->data.int8[i] = quantized_value[i];
	}
	/* Now put quantized data on input of NN */
	// input->data.int8[0] = *quantized_value;
	/* Process NN */
	// GPIO->P[gpioPortB].DOUT |= 1<<9;

	myStatus = ProcessInference();
	if(myStatus != kTfLiteOk){
		while(1);
	}

	// GPIO->P[gpioPortB].DOUT &= ~(1<<9);

	// // int8_t final_output;
	// // std::copy_n(tflite::GetTensorData<int8_t>(output), 1, &final_output);
	int8_t final_output = output->data.int8[0];
	float output_score = static_cast<float>(static_cast<int>(final_output) - output_zero_point) * output_scale;
	// GPIO->P[gpioPortA].DOUT &= ~(1<<7);
	if(output_score > 0.75f){
		ActivateTask(write_audio);
		GPIO->P[gpioPortA].DOUT &= ~(1<<8);

	}
	ChainTask(inference);
	// TerminateTask();
}
#define APP_Task_inference_STOP_SEC_CODE
#include "tpl_memmap.h"