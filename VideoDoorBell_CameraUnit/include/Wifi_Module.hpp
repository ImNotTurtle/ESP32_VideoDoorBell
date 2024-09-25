#ifndef WIFI_MODULE_HPP
#define WIFI_MODULE_HPP
#include <WiFi.h>
#include <functional>


class Wifi_Module{
public: 
  static void Init();
  static bool ConfigStaticIP(IPAddress localIP, IPAddress gateway, IPAddress subnet);
  static void Connect(String ssid, String pw, std::function<void()> onSuccess, std::function<void()> onFailure = NULL);
  static void StopConnect(void);
  static String GetLocalIP(void);
  static String GetAPIP(void);
  static String GetPublicIP();
  static bool ConfigAccessPoint(String ssid_ap, String pw_ap);

  

  static int m_reconnectCount;
  static bool m_stopConnect;
private:
  static IPAddress m_localIP;  // Địa chỉ IP tĩnh cho ESP32-CAM
  static IPAddress m_gateway;    // Địa chỉ IP của router
  static IPAddress m_subnet;   // Subnet Mask
};



#endif