#include "esp_camera.h"
camera_config_t camera_config = {
    .pin_pwdn       = -1,
    .pin_reset      = 18,
    .pin_xclk       = 14,
    
    .pin_sccb_sda   = 4,
    .pin_sccb_scl   = 5,
   
    .pin_d7         = 15,
    .pin_d6         = 16,
    .pin_d5         = 17,
    .pin_d4         = 12,
    .pin_d3         = 10,
    .pin_d2         = 8,
    .pin_d1         = 9,
    .pin_d0         = 11,
    .pin_vsync      = 6,
    .pin_href       = 7,
    .pin_pclk       = 13,
  
    .xclk_freq_hz   = 15000000,
    .pixel_format   = PIXFORMAT_JPEG,//RGB565, //PIXFORMAT_RGB565,  // For image capture
    .frame_size     = FRAMESIZE_HD, //FRAMESIZE_96X96,  // Choose based on your need
  
    .jpeg_quality   = 15,
    .fb_count       = 1,


    .fb_location    = CAMERA_FB_IN_DRAM // Use PSRAM for frame buffer
    //.fb_location   = CAMERA_FB_IN_PSRAM, // Use PSRAM for frame buffer
    };