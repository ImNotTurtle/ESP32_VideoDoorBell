#include "Wifi_Module.hpp"
#include "Camera_Module.hpp"
#include "WebSocket_Module.hpp"
// #include "WebServer_Module.hpp"

const char *ssid_AP = "ESP32-CAM-AP";
const char *password_AP = "12345678";
IPAddress localIP(192, 168, 1, 50);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
SemaphoreHandle_t interruptTriggerSem;

void deepSleep(String reason);
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length); // WebSocket event callback

void setup()
{
  Serial.begin(57600);

  Serial.setDebugOutput(true);
  // config static IP to use port forwarding
  if (Wifi_Module::ConfigStaticIP(localIP, gateway, subnet))
  {
    Serial.println("Wifi_Module : Config static IP success");
  }
  else
  {
    Serial.println("Wifi_Module : Config static IP failed");
  }
  Wifi_Module::Init();
  bool connected = false;
  Wifi_Module::Connect(
      [&]() { // on wifi connection success
        connected = true;
      },
      []()
      {
        deepSleep("Enter deep sleep due to wifi connection fail.");
      });

  if (connected)
  {
    if (Wifi_Module::ConfigAccessPoint(ssid_AP, password_AP))
    {
      Serial.printf("Wifi_Module : Config AccessPoint success with ssid: %s, password: %s\n", ssid_AP, password_AP);
    }
    else
    {
      Serial.println("Wifi_Module : Config AccessPoint fail");
    }
    // init camera and web server
    Serial.println("Wifi_Module : Connected to WiFi. ");
    Serial.printf("\tIP STA: %s. \n", Wifi_Module::GetLocalIP());
    Serial.printf("\tIP AP: %s\n", Wifi_Module::GetAPIP());
    Serial.printf("\tIP Modem public IP: %s\n", Wifi_Module::GetPublicIP().c_str());
    Serial.printf("\tRSSI: %d\n", Wifi_Module::GetRSSI());

    Camera_Module::Init();
    Serial.println("Camera_Module : Init success");

    WebSocket_Module::Init();
    Serial.println("WebSocket_Module : Init success");
    WebSocket_Module::Begin(onWebSocketEvent);
  }
}

void loop()
{
  WebSocket_Module::Loop();
}

// WebSocket event callback
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  if (type == WStype_CONNECTED)
  {
    // Serial.printf("WebSocket_Module : Client connected: %u\n", num);
  }
  else if (type == WStype_DISCONNECTED)
  {
    // Serial.printf("WebSocket_Module : Client disconnected: %u\n", num);
  }
  else if (type == WStype_TEXT)
  {
    String message = (char *)payload;
    if (message == "RingBell")
    { // receive RingDoorBell message from web socket client
      Serial.println("Ding Doongggggg");
      // send a signal to monitor unit to play bell sound
      WebSocket_Module::BroadcastBell();
    }
  }
  else if (type == WStype_PING)
  {
    Serial.println("Ping");
  }
  else if (type == WStype_PONG)
  {
    Serial.println("Pong");
  }
  else
  {
    Serial.printf("Websocket: Received type: %d\n", type);
  }
}

void deepSleep(String reason)
{
  Serial.println(reason);
  esp_deep_sleep_start();
}
