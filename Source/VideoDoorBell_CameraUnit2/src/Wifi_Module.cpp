#include "../include/Wifi_Module.hpp"
#include <HTTPClient.h>

IPAddress Wifi_Module::m_localIP = IPAddress(0, 0, 0, 0); // Địa chỉ IP tĩnh cho ESP32-CAM
IPAddress Wifi_Module::m_gateway = IPAddress(0, 0, 0, 0); // Địa chỉ IP của router
IPAddress Wifi_Module::m_subnet = IPAddress(0, 0, 0, 0);  // Subnet Mask
int Wifi_Module::m_reconnectCount = 0;
bool Wifi_Module::m_stopConnect = false;

bool Wifi_Module::ConfigStaticIP(IPAddress localIP, IPAddress gateway, IPAddress subnet)
{
  m_localIP = localIP;
  m_gateway = gateway;
  m_subnet = subnet;
  return WiFi.config(m_localIP, m_gateway, m_subnet, IPAddress(8, 8, 8, 8));
}
void Wifi_Module::Init()
{
}
void Wifi_Module::Connect(std::function<void()> onSuccess, std::function<void()> onFailure)
{
  // WiFiManager wm;
  // wm.setConfigPortalTimeout(180); // 3 minutes
  // wm.setEnableConfigPortal(true); // execute portal config when wifi connection is failed
  // if(onFailure != NULL){
  //   wm.setConfigPortalTimeoutCallback(onFailure);
  // }

  // // connect to saved wifi
  // wm.autoConnect("ESP32_CAM-WifiManager"); // generate Access Point to change wifi config

  // Serial.println("Connected to wifi: ");
  // Serial.printf("\tSSID: %s\n", wm.getWiFiSSID());
  // Serial.printf("\tPASS: %s\n", wm.getWiFiPass());

  WiFi.begin("Duyen Pham 1", "99999999");
  while(WiFi.status() != WL_CONNECTED){
    delay(1000);
  }

  if(onSuccess != NULL){
    onSuccess();
  }
}
void Wifi_Module::StopConnect(void)
{
  m_stopConnect = true;
}
String Wifi_Module::GetLocalIP(void)
{
  return WiFi.localIP().toString();
}
String Wifi_Module::GetAPIP(void)
{
  return WiFi.softAPIP().toString();
}
String Wifi_Module::GetPublicIP()
{
  HTTPClient http;
  http.begin("http://api.ipify.org");
  int httpCode = http.GET();
  String ip = "";
  if (httpCode > 0)
  {
    ip = http.getString();
  }
  http.end();
  return ip;
}
bool Wifi_Module::ConfigAccessPoint(String ssid_ap, String pw_ap)
{
  return WiFi.softAP(ssid_ap, pw_ap);
}
bool Wifi_Module::IsDisconnected(void){
  return WiFi.status() != WL_CONNECTED;
}
int32_t Wifi_Module::GetRSSI(void){
  return WiFi.RSSI();
}