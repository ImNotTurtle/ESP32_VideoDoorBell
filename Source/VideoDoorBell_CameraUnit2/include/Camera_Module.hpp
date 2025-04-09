#ifndef CAMERA_MODULE_HPP
#define CAMERA_MODULE_HPP

// Cấu hình IP tĩnh
#include "esp_camera.h"
#include <functional>

class Camera_Module{
public:
  Camera_Module();

  static void Init(void);
  static void StartServer(void);
  static void CaptureImage(std::function<void(uint8_t* buffer, size_t len)> handle, std::function<void()> onFail = NULL);

private:

};


#endif