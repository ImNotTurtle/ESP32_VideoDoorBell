#include "../include/Camera_Module.hpp"

#define CAMERA_MODEL_AI_THINKER // Has PSRAM

#include "HardwareSerial.h"
#include "../include/camera_pins.h"

extern HardwareSerial Serial;
extern bool psramFound(void);
extern void startCameraServer();
extern void setupLedFlash(int pin);

#define CAM_PIN_PWDN 32
#define CAM_PIN_RESET -1 // software reset will be performed
#define CAM_PIN_XCLK 0
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27

#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 21
#define CAM_PIN_D2 19
#define CAM_PIN_D1 18
#define CAM_PIN_D0 5
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22
// static camera_config_t camera_config = {
//     .pin_pwdn = CAM_PIN_PWDN,
//     .pin_reset = CAM_PIN_RESET,
//     .pin_xclk = CAM_PIN_XCLK,
//     .pin_sccb_sda = CAM_PIN_SIOD,
//     .pin_sccb_scl = CAM_PIN_SIOC,

//     .pin_d7 = CAM_PIN_D7,
//     .pin_d6 = CAM_PIN_D6,
//     .pin_d5 = CAM_PIN_D5,
//     .pin_d4 = CAM_PIN_D4,
//     .pin_d3 = CAM_PIN_D3,
//     .pin_d2 = CAM_PIN_D2,
//     .pin_d1 = CAM_PIN_D1,
//     .pin_d0 = CAM_PIN_D0,
//     .pin_vsync = CAM_PIN_VSYNC,
//     .pin_href = CAM_PIN_HREF,
//     .pin_pclk = CAM_PIN_PCLK,

//     // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
//     .xclk_freq_hz = 20000000,
//     .ledc_timer = LEDC_TIMER_0,
//     .ledc_channel = LEDC_CHANNEL_0,

//     .pixel_format = PIXFORMAT_JPEG, // YUV422,GRAYSCALE,RGB565,JPEG
//     .frame_size = FRAMESIZE_QVGA,   // QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.

//     .jpeg_quality = 12, // 0-63, for OV series camera sensors, lower number means higher quality
//     .fb_count = 1,      // When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
//     .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
// };

// Camera_Module::Camera_Module()
// {
// }

// void Camera_Module::Init(void)
// {
//   esp_err_t err = esp_camera_init(&camera_config);

//   if (err != ESP_OK)
//   {
//     // Serial.printf("Camera init failed with error 0x%x\n", err);
//     return;
//   }

//   sensor_t *s = esp_camera_sensor_get();
//   if (s)
//   {
//     s->set_vflip(s, 1); // 1 để bật VFlip, 0 để tắt
//   }
// }
static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    .xclk_freq_hz = 20000000, // 20MHz, giúp cải thiện tốc độ khung hình
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG, // Chế độ JPEG để giảm tải xử lý
    .frame_size = FRAMESIZE_QVGA,   // 800x600. Nếu hệ thống bị đơ, cân nhắc giảm xuống FRAMESIZE_VGA (640x480)
    .jpeg_quality = 10,             // JPEG quality: giá trị nhỏ hơn cho chất lượng cao hơn
    .fb_count = 2,                  // Số framebuffer: có thể thử tăng lên 2 hoặc 3 nếu cần
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_LATEST,
};

Camera_Module::Camera_Module()
{
}

void Camera_Module::Init(void)
{
  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK)
  {
    // Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  if (s)
  {
    // Điều chỉnh hiển thị: lật dọc & ngang nếu cần
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);

    // Cài đặt độ sáng và màu
    s->set_brightness(s, 1);  // Điều chỉnh độ sáng: thử nghiệm với giá trị 0 nếu 1 làm ảnh bị sáng màu
    s->set_contrast(s, 1);    // Giữ độ tương phản trung tính, có thể điều chỉnh nếu cần
    s->set_saturation(s, 0); // Giảm saturation có thể giúp hạn chế hiện tượng "vàng"
    s->set_sharpness(s, 1);   // Độ nét: giữ mức trung tính

    // Cân bằng trắng & phơi sáng
    s->set_awb_gain(s, 0);                   // Tắt AWB gain nếu gặp vấn đề màu vàng
    s->set_wb_mode(s, 1);                    // 0: chế độ tự động, có thể thử thay đổi nếu cần (ví dụ, 1: sunny)
    s->set_exposure_ctrl(s, 1);              // Bật tự động phơi sáng
    s->set_aec_value(s, 1000);                // Giá trị phơi sáng: điều chỉnh thử nếu ảnh quá sáng/tối
    s->set_agc_gain(s, 1);                   // Điều chỉnh gain tự động
    s->set_gainceiling(s, (gainceiling_t)6); // Giới hạn gain tối đa

    // Bật sửa lỗi ống kính để giảm méo ảnh
    s->set_lenc(s, 0);
  }
}

void Camera_Module::StartServer(void)
{
  // startCameraServer();
}

void Camera_Module::CaptureImage(std::function<void(uint8_t *buffer, size_t len)> handle, std::function<void()> onFail)
{
  camera_fb_t *fb = esp_camera_fb_get();
  if (fb && handle)
    handle(fb->buf, fb->len); // capture success then call the handle
  else if (onFail)
    onFail(); // capture fail
  esp_camera_fb_return(fb);
}