#include "../include/WebSocket_Module.hpp"
#include "../include/Camera_Module.hpp"

#include "Base64.hpp"
#include "base64.h"

#ifndef WEB_SOCKET_DEFAULT_PORT
#define WEB_SOCKET_DEFAULT_PORT 81
#endif

static WebSocketsServer webSocket(WEB_SOCKET_DEFAULT_PORT);

void WebSocket_Module::Init()
{
    Serial.printf("WebSocket_Module : Init with port: %d\n", WEB_SOCKET_DEFAULT_PORT);
}
void WebSocket_Module::Begin(std::function<void(uint8_t num, WStype_t type, uint8_t *payload, size_t length)> eventHandle)
{
    webSocket.begin();
    webSocket.onEvent(eventHandle);

    xTaskCreate(
        [](void *pvParameters)
        {
            //delay for initialization
            vTaskDelay(pdMS_TO_TICKS(5000));
            uint8_t count = 0;

            while (true)
            {
                WebSocket_Module::BroadcastImage();
                vTaskDelay(pdMS_TO_TICKS(1000 / 30)); // 33ms between frames, 30FPS
                count++;
                if(count % 100 == 0){
                    //Serial.printf("WebSocket_Module:streamTask: Never used stack space: %u\n", uxTaskGetStackHighWaterMark(NULL));
                    //1420 min
                }
            }
            // clean up
            vTaskDelete(NULL);
        },
        "WebSocket_Module:streamTask", 1024 * 3, NULL, 1, NULL);
}

void WebSocket_Module::Loop()
{
    webSocket.loop();
}

void WebSocket_Module::BroadcastImage()
{
    Camera_Module::CaptureImage([](uint8_t *buffer, size_t len) { // capture image success
        webSocket.broadcastBIN(buffer, len);
    });
}

void WebSocket_Module::BroadcastBell(){
    webSocket.broadcastTXT("RingBell");
}