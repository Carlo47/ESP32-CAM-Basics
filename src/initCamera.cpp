#include <Arduino.h>
#include <esp_camera.h>

// Pin definition for CAMERA_MODEL_AI_THINKER
// Change pin definition if you are using another ESP32 camera module
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

/**
 * Initialize the camera
*/
void initCamera()
{
  // Stores the camera configuration parameters
  camera_config_t camConfig = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sccb_sda = SIOD_GPIO_NUM,
    .pin_sccb_scl = SIOC_GPIO_NUM,
    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,

    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    
    .pixel_format = PIXFORMAT_JPEG,   // YUV422, GRAYSCALE, RGB565, JPEG
    .grab_mode = CAMERA_GRAB_LATEST,  // CAMERA_GRAB_WHEN_EMPTY
  };

  // Select lower framesize if the camera doesn't support PSRAM
  if(psramFound())
  {
    camConfig.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    camConfig.jpeg_quality = 10; //0-63 lower number means higher quality
    camConfig.fb_count = 1;
  } else 
  {
    camConfig.frame_size = FRAMESIZE_SVGA;
    camConfig.jpeg_quality = 12;
    camConfig.fb_count = 1;
  }
  
  // Initialize the Camera
  while (esp_err_t err = esp_camera_init(&camConfig) != ESP_OK) 
  {
    log_i("...failed with error 0x%x", err);
    delay(1000);
  }
  log_i("==> done");
}
