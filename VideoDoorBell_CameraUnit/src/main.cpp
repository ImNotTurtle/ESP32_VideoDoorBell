#include "Wifi_Module.hpp"
#include "Camera_Module.hpp"
#include "WebSocket_Module.hpp"
#include "WebServer_Module.hpp"

// Thông tin mạng Wi-Fi
const char *ssid = "My-wifi-ssid";
const char *password = "My-wifi-password";
const char *ssid_AP = "ESP32-CAM-AP"; // Tên Wi-Fi AP của ESP32-CAM
const char *password_AP = "12345678"; // Mật khẩu Wi-Fi AP
IPAddress localIP(192, 168, 1, 50);   // Địa chỉ IP tĩnh cho ESP32-CAM
IPAddress gateway(192, 168, 1, 1);    // Địa chỉ IP của router
IPAddress subnet(255, 255, 255, 0);   // Subnet Mask

bool wifiInitSuccess = false;
// WebSocket callback
// see #WebSocketEventHeader in WebServer_Module.cpp to handle properly
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_CONNECTED) {
    Serial.printf("WebSocket_Module : Client connected: %u\n", num);
  } 
  else if (type == WStype_DISCONNECTED) {
    Serial.printf("WebSocket_Module : Client disconnected: %u\n", num);
  }
  else if (type == WStype_TEXT) {
    String message = (char*)payload;
    if (message == "RingDoorBell") {
      Serial.println("Ding Doongggggg");
      //play door bell sound
    }
  }
}

void setup() {
  // Khởi tạo Serial
  Serial.begin(9600);
  Serial.setDebugOutput(true);
  if (Wifi_Module::ConfigStaticIP(localIP, gateway, subnet)) {
    Serial.println("Wifi_Module : Config static IP success");
  }
  else Serial.println("Wifi_Module : Config static IP failed");
  // Khởi tạo chế độ Station (STA)
  Wifi_Module::Init();
  Wifi_Module::Connect(
    ssid,
    password,
    []() { // on wifi connection success
      if(Wifi_Module::ConfigAccessPoint(ssid_AP, password_AP)){
        Serial.printf("Wifi_Module : Config AccessPoint success with ssid: %s, password: %s\n", ssid_AP, password_AP);
      }
      else{
        Serial.println("Wifi_Module : Config AccessPoint fail");
      }
      // init camera and web server 
      Serial.println("Wifi_Module : Connected to WiFi. ");
      Serial.printf("\tIP STA: %s. \n", Wifi_Module::GetLocalIP());
      Serial.printf("\tIP AP: %s\n", Wifi_Module::GetAPIP());
      Serial.printf("\tIP Modem public IP: %s\n", Wifi_Module::GetPublicIP());

      Camera_Module::Init();
      Serial.println("Camera_Module : Init success");

      WebServer_Module::Init();
      Serial.println("WebServer_Module : Init success");
      WebServer_Module::Begin();

      WebSocket_Module::Init();
      Serial.println("WebSocket_Module : Init success");
      WebSocket_Module::Begin(onWebSocketEvent);
      wifiInitSuccess = true;
    },
    []() { // on wifi connection failure
      Serial.printf("Wifi_Module : Connect to wifi fail. Attemp count: %d.\n", Wifi_Module::m_reconnectCount);
      if (Wifi_Module::m_reconnectCount > 30)
      { // so much effort to estashlish wifi connection
        Serial.println("Wifi_Module : Can not establish wifi connection, Wifi_Module will not function");
        Wifi_Module::StopConnect();
        wifiInitSuccess = false;
      }
    });

}

void loop() {
  //wifi connection established
  if(wifiInitSuccess){
    WebSocket_Module::Loop();
  }
}
