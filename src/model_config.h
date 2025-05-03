#if MODEL == 1
#include "1_model_data.h"
#define MODEL_DATA __1_int8_tflite
#define MODEL_LEN __1_int8_tflite_len
#define MODEL_NAME "Model 1"
#define INPUT_HEIGHT 96
#define INPUT_WIDTH 96
#define INPUT_CHANNELS 1
#elif MODEL == 2
#include "2_model_data.h"
#define MODEL_DATA __2_int8_tflite
#define MODEL_LEN __2_int8_tflite_len
#define MODEL_NAME "Model 2"
#define INPUT_HEIGHT 96
#define INPUT_WIDTH 96
#define INPUT_CHANNELS 3
#elif MODEL == 3
#include "3_model_data.h"
#define MODEL_DATA __3_int8_tflite
#define MODEL_LEN __3_int8_tflite_len
#define MODEL_NAME "Model 3"
#define INPUT_HEIGHT 120
#define INPUT_WIDTH 160
#define INPUT_CHANNELS 1
#elif MODEL == 4
#include "4_model_data.h"
#define MODEL_DATA __4_int8_tflite
#define MODEL_LEN __4_int8_tflite_len
#define MODEL_NAME "Model 4"
#define INPUT_HEIGHT 120
#define INPUT_WIDTH 160
#define INPUT_CHANNELS 3
#elif MODEL == 5
#include "5_model_data.h"
#define MODEL_DATA __5_int8_tflite
#define MODEL_LEN __5_int8_tflite_len
#define MODEL_NAME "Model 5"
#define INPUT_HEIGHT 96
#define INPUT_WIDTH 96
#define INPUT_CHANNELS 1
#elif MODEL == 6
#include "6_model_data.h"
#define MODEL_DATA __6_int8_tflite
#define MODEL_LEN __6_int8_tflite_len
#define MODEL_NAME "Model 6"
#define INPUT_HEIGHT 96
#define INPUT_WIDTH 96
#define INPUT_CHANNELS 3
#elif MODEL == 7
#include "7_model_data.h"
#define MODEL_DATA __7_int8_tflite
#define MODEL_LEN __7_int8_tflite_len
#define MODEL_NAME "Model 7"
#define INPUT_HEIGHT 120
#define INPUT_WIDTH 160
#define INPUT_CHANNELS 1
#elif MODEL == 8
// #include "8_model_data.h"
// #define MODEL_DATA __8_int8_tflite
// #define MODEL_LEN __8_int8_tflite_len
// #define MODEL_NAME "Model 8"
// #define INPUT_HEIGHT 120
// #define INPUT_WIDTH 160
// #define INPUT_CHANNELS 3
#include "4_model_data.h"
#define MODEL_DATA __4_int8_tflite
#define MODEL_LEN __4_int8_tflite_len
#define MODEL_NAME "Model 4"
#define INPUT_HEIGHT 120
#define INPUT_WIDTH 160
#define INPUT_CHANNELS 3

#else
#error "No model defined"
#endif

