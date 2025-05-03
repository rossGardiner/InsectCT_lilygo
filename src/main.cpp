#include <Arduino.h>
#include "esp_camera.h"
#include "camera_config.h"
#include "JPEGDecoder.h"

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include "esp_log.h"
#include "esp_heap_caps.h"

#include "esp_vfs_fat.h"

#include "model_config.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"


// // Globals, used for compatibility with Arduino-style sketches.
namespace {
  tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* input = nullptr;
  
  constexpr int scratchBufSize = 80 * 1024;
  //constexpr int kTensorArenaSize = 275 * 1024 + scratchBufSize;
  const int kTensorArenaSize = 2*MODEL_LEN + scratchBufSize;
  static uint8_t *tensor_arena;
  static uint8_t* model_data = nullptr;
  // Global: pre-allocated DRAM buffer (0.1MB safe for most JPEGs)
  uint8_t saved_frame_buf[100 * 1024];
  size_t saved_frame_len = 0;
  volatile bool image_ready_for_save = false;
  char current_filename[32] = "/saved.jpg";
  SemaphoreHandle_t saveSemaphore;

  }  // namespace
  
// SPI pin definitions (adjust if needed)
#define SD_CS    GPIO_NUM_47 
#define SD_MOSI  GPIO_NUM_38
#define SD_MISO  GPIO_NUM_40
#define SD_CLK   GPIO_NUM_39 
#define SD_D0 SD_MOSI
#define SD_CMD SD_MISO

//SAVE
#define SAVE 1

#if defined(COLLECT_CPU_STATS)
extern long long softmax_total_time;
extern long long dc_total_time;
extern long long conv_total_time;
extern long long fc_total_time;
extern long long pooling_total_time;
extern long long add_total_time;
extern long long mul_total_time;
#endif

void captureTask(void *pvParameters);
void runInference();
bool preprocessFrame(camera_fb_t* fb, TfLiteTensor* input);

// ----- SD Task -----
void storageTask(void* pvParameters) {
  while (true) {
    
    if (xSemaphoreTake(saveSemaphore, portMAX_DELAY) == pdTRUE) {
        char full_path[64];
        snprintf(full_path, sizeof(full_path), "%s", current_filename);
        File file = SD.open(full_path, FILE_WRITE);
        if (file) {
            file.write(saved_frame_buf, saved_frame_len);   
            file.close();
            Serial.printf("Image written to %s (%d bytes)\n", full_path, saved_frame_len);
        } else {
          Serial.println("SD write failed");
        }
    } 
  }
}


void setup() {
    
  
    Serial.begin(115200);
    delay(4000);
    Serial.println("Device booted");
    Serial.printf("Internal RAM: %d bytes\n", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    Serial.printf("PSRAM:        %d bytes\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    //init sd card
    Serial.println("Initializing SD card...");
    pinMode(SD_CS, OUTPUT); // Set CS pin as output
    digitalWrite(SD_CS, HIGH);  // toggle high to reset connection
    delay(500);
    SPI.begin(SD_CLK, SD_MISO, SD_MOSI, SD_CS);
    SPI.setDataMode(SPI_MODE0);

    if (!SD.begin(SD_CS, SPI, 80000000)) {
        Serial.println("SD card init failed!");
        while (true);  // Halt
    }
    Serial.println("SD card initialized using SPI");

    delay(2000);
    //init camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        while (true);  // Halt
    }

    Serial.println("Initializing TensorFlow Lite model...");

    model = tflite::GetModel(MODEL_DATA);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        Serial.printf("Model provided is schema version %d not equal to supported version %d. \n", model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    if (tensor_arena == NULL) {
        Serial.printf("Requesting memory of %d bytes\n", kTensorArenaSize);
        tensor_arena = (uint8_t *) heap_caps_malloc(kTensorArenaSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    }
    if (tensor_arena == NULL) {
        Serial.printf("Couldn't allocate memory of %d bytes\n", kTensorArenaSize);
        return;
    }

    static tflite::MicroMutableOpResolver<9> micro_op_resolver;
    micro_op_resolver.AddConv2D();
    micro_op_resolver.AddDepthwiseConv2D();
    micro_op_resolver.AddQuantize();
    micro_op_resolver.AddDequantize();
    micro_op_resolver.AddAdd();
    micro_op_resolver.AddPad();
    micro_op_resolver.AddMean();
    micro_op_resolver.AddFullyConnected();
    micro_op_resolver.AddLogistic();

    static tflite::MicroInterpreter static_interpreter(model, micro_op_resolver, tensor_arena, kTensorArenaSize);
    interpreter = &static_interpreter;

    // Allocate memory from the tensor_arena for the model's tensors.
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
    Serial.println("AllocateTensors() failed");
    return;
    }

    // Get information about the memory area to use for the model's input.
    input = interpreter->input(0);
    saveSemaphore = xSemaphoreCreateBinary();

    xTaskCreatePinnedToCore(storageTask, "StorageTask", 4096, NULL, 1, NULL, 0);



  }
  

  int current_file_index = 0;

  void loop() {
    Serial.printf("Internal RAM: %d bytes\n", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    Serial.printf("PSRAM:        %d bytes\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    unsigned long t0 = millis();
  
    // 1. Capture JPEG
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb || fb->format != PIXFORMAT_JPEG) {
      Serial.println("Invalid camera frame");
      return;
    }
    Serial.printf("JPEG captured (%d bytes)\n", fb->len);
  
    // 2. Decode + resize
    unsigned long t1 = millis();
    JpegDec.decodeArray(fb->buf, fb->len);
    int src_w = JpegDec.width, src_h = JpegDec.height;
    float x_ratio = (float)src_w / INPUT_WIDTH;
    float y_ratio = (float)src_h / INPUT_HEIGHT;
    int input_idx = 0;
  
    for (int y = 0; y < INPUT_HEIGHT; y++) {
      for (int x = 0; x < INPUT_WIDTH; x++) {
        int src_x = int(x * x_ratio);
        int src_y = int(y * y_ratio);
        int src_idx = (src_y * src_w + src_x) * JpegDec.comps;
  
        uint8_t r = JpegDec.pImage[src_idx + 0];
        uint8_t g = (JpegDec.comps > 1) ? JpegDec.pImage[src_idx + 1] : r;
        uint8_t b = (JpegDec.comps > 1) ? JpegDec.pImage[src_idx + 2] : r;
  
        if (INPUT_CHANNELS == 1) {
          input->data.int8[input_idx++] = ((r + g + b) / 3) - 128;
        } else {
          input->data.int8[input_idx++] = r - 128;
          input->data.int8[input_idx++] = g - 128;
          input->data.int8[input_idx++] = b - 128;
        }
      }
    }
    Serial.printf("Decode + resize done in %lu ms\n", millis() - t1);
  
    // 3. Inference
    t1 = millis();
    if (interpreter->Invoke() != kTfLiteOk) {
      Serial.println("Inference failed");
    } else {
      int8_t score = interpreter->output(0)->data.int8[0];
      Serial.printf("Inference done in %lu ms, score: %d\n", millis() - t1, score);
  
      // 4. If interesting, copy to DRAM for SD task
      if (SAVE && !image_ready_for_save) {
        memcpy(saved_frame_buf, fb->buf, fb->len);
        saved_frame_len = fb->len;
        current_file_index = (current_file_index + 1) % 10;
        snprintf(current_filename, sizeof(current_filename), "/photo_%d.jpg", current_file_index);
        xSemaphoreGive(saveSemaphore);
      }
    }
  
    esp_camera_fb_return(fb);
    Serial.printf("⏱ Total loop time: %lu ms\n\n", millis() - t0);
    //Serial.printf("Running on core %d\n", xPortGetCoreID());

    //delay(50);
  }
  

void captureTask(void *pvParameters) {
  while (true) {
    Serial.println("Capturing image (Core 1)...");

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      delay(2000);
      continue;
    }

    File file = SD.open("/photo.dat", FILE_WRITE);
    if (!file) {
      Serial.println("Failed to open file");
      esp_camera_fb_return(fb);
      delay(2000);
      continue;
    }

    file.write(fb->buf, fb->len);
    file.close();
    esp_camera_fb_return(fb);

    Serial.printf("Saved image (%d bytes)\n", fb->len);

    
  }
}


bool preprocessFrame(camera_fb_t* fb, TfLiteTensor* input) {
  if (!fb || fb->format != PIXFORMAT_RGB565) {
    Serial.println("Frame invalid or not in RGB565 format");
    return false;
  }

  int src_w = fb->width;
  int src_h = fb->height;
  const uint16_t* src = (uint16_t*)fb->buf;

  float x_ratio = (float)src_w / INPUT_WIDTH;
  float y_ratio = (float)src_h / INPUT_HEIGHT;

  int input_idx = 0;

  for (int y = 0; y < INPUT_HEIGHT; y++) {
    for (int x = 0; x < INPUT_WIDTH; x++) {
      int src_x = int(x * x_ratio);
      int src_y = int(y * y_ratio);
      uint16_t rgb565 = src[src_y * src_w + src_x];

      uint8_t r = ((rgb565 >> 11) & 0x1F) << 3;
      uint8_t g = ((rgb565 >> 5) & 0x3F) << 2;
      uint8_t b = (rgb565 & 0x1F) << 3;

      if (INPUT_CHANNELS == 1) {
        uint8_t gray = (r + g + b) / 3;
        input->data.int8[input_idx++] = gray - 128;  // if quantized
      } else {
        input->data.int8[input_idx++] = r - 128;
        input->data.int8[input_idx++] = g - 128;
        input->data.int8[input_idx++] = b - 128;
      }
    }
  }

  return true;
}

// bool decodeAndPreprocessJpg(const char* path, TfLiteTensor* input) {
//     JpegDec.decodeSdFile(path);
//     if (!JpegDec.read()) {
//       Serial.println("JPEG decode failed");
//       return false;
//     }
  
//     int src_w = JpegDec.width;
//     int src_h = JpegDec.height;
  
//     float x_ratio = (float)src_w / INPUT_WIDTH;
//     float y_ratio = (float)src_h / INPUT_HEIGHT;
  
//     int input_idx = 0;
  
//     for (int y = 0; y < INPUT_HEIGHT; y++) {
//       for (int x = 0; x < INPUT_WIDTH; x++) {
//         int src_x = int(x * x_ratio);
//         int src_y = int(y * y_ratio);
//         uint16_t color = JpegDec.getPixel(src_x, src_y);  // RGB565
  
//         uint8_t r = ((color >> 11) & 0x1F) << 3;
//         uint8_t g = ((color >> 5) & 0x3F) << 2;
//         uint8_t b = (color & 0x1F) << 3;
  
//         if (INPUT_CHANNELS == 1) {
//           uint8_t gray = (r + g + b) / 3;
//           input->data.int8[input_idx++] = gray - 128;
//         } else {
//           input->data.int8[input_idx++] = r - 128;
//           input->data.int8[input_idx++] = g - 128;
//           input->data.int8[input_idx++] = b - 128;
//         }
//       }
//     }
  
//     return true;
//   }

void runInference() {
  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("Inference failed!");
    return;
  }

  TfLiteTensor* output = interpreter->output(0);
  Serial.print("Inference output: ");
  for (int i = 0; i < output->bytes && i < 5; i++) {
    Serial.printf("%d ", output->data.int8[i]);
  }
  Serial.println();

  #if defined(COLLECT_CPU_STATS)
  Serial.printf("Operator timings (µs):\n");
  Serial.printf("  Conv2D       : %lld\n", conv_total_time);
  Serial.printf("  FullyConnected: %lld\n", fc_total_time);
  Serial.printf("  DepthwiseConv: %lld\n", dc_total_time);
  Serial.printf("  Pooling      : %lld\n", pooling_total_time);
  Serial.printf("  Add          : %lld\n", add_total_time);
  Serial.printf("  Mul          : %lld\n", mul_total_time);
  Serial.printf("  Softmax      : %lld\n", softmax_total_time);
#endif

}