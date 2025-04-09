#include "../include/WebSocket_Module.hpp"
#include "../include/Camera_Module.hpp"
#include <freertos/semphr.h>

#ifndef WEB_SOCKET_DEFAULT_PORT
#define WEB_SOCKET_DEFAULT_PORT 81
#endif

#define STREAM_FPS 15

// Khởi tạo đối tượng WebSocketsServer và mutex để đồng bộ truy cập
static WebSocketsServer webSocket(WEB_SOCKET_DEFAULT_PORT);
static SemaphoreHandle_t wsMutex = NULL;

void streamTask(void *arg);

// Global flag để theo dõi trạng thái kết nối của client
static volatile uint8_t clientCount = 0;

void WebSocket_Module::Init()
{
    Serial.printf("WebSocket_Module : Init with port: %d\n", WEB_SOCKET_DEFAULT_PORT);
    // Tạo mutex để bảo vệ các thao tác trên webSocket
    wsMutex = xSemaphoreCreateMutex();
    if (wsMutex == NULL)
    {
        Serial.println("WebSocket_Module: Failed to create mutex");
    }
}

int count = 0;
void WebSocket_Module::Begin(std::function<void(uint8_t num, WStype_t type, uint8_t *payload, size_t length)> eventHandle)
{
    // Khởi tạo webSocket và thiết lập callback
    webSocket.begin();
    webSocket.onEvent([eventHandle](uint8_t num, WStype_t type, uint8_t *payload, size_t length)
                      {
        // Cập nhật flag trạng thái kết nối dựa theo sự kiện
        if (type == WStype_CONNECTED) {
            // clientConnected = true;
            clientCount++;
            Serial.printf("WebSocket_Module : Client connected: %u\n", num);
        }
        else if (type == WStype_DISCONNECTED) {
            // clientConnected = false;
            if(clientCount > 0){
                clientCount--;
            }
            Serial.printf("WebSocket_Module : Client disconnected: %u\n", num);
        }
        // Gọi callback do người dùng định nghĩa
        eventHandle(num, type, payload, length); });

    // Tạo task gửi dữ liệu (ảnh) qua WebSocket
    xTaskCreate(
        streamTask,
        "WebSocket_Module:streamTask", 1024 * 3, NULL, 1, NULL);
}

void WebSocket_Module::Loop()
{
    webSocket.loop();
}

void WebSocket_Module::BroadcastImage()
{
    // Capture hình ảnh và gửi qua webSocket nếu có client kết nối
    Camera_Module::CaptureImage([](uint8_t *buffer, size_t len)
                                {
        if (xSemaphoreTake(wsMutex, portMAX_DELAY) == pdTRUE) {
        webSocket.broadcastBIN(buffer, len);

        Serial.printf("Broadcast image out %d with length: %d\n", count++, len);
        // webSocket.broadcastTXT("RingBell");
        xSemaphoreGive(wsMutex);
        } });
}

void WebSocket_Module::BroadcastBell()
{
    // Gửi tín hiệu "RingBell" cho client
    if (xSemaphoreTake(wsMutex, portMAX_DELAY) == pdTRUE)
    {
        webSocket.broadcastTXT("RingBell", 8);
        xSemaphoreGive(wsMutex);
    }
}

void streamTask(void *arg)
{
    // Đợi khởi tạo ban đầu
    vTaskDelay(pdMS_TO_TICKS(5005));
    while (true)
    {
        // Chỉ gửi dữ liệu khi có client kết nối
        if (clientCount > 0)
        {
            WebSocket_Module::BroadcastImage();
        }

        vTaskDelay(pdMS_TO_TICKS(1000 / STREAM_FPS));
    }
    vTaskDelete(NULL);
}