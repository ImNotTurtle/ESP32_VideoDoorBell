#ifndef WEBSOCKET_MODULE_H
#define WEBSOCKET_MODULE_H

#include <WebSocketsServer.h>
#include "esp_camera.h"
#include <functional>

#define WEB_SOCKET_DEFAULT_PORT 81

class WebSocket_Module {
public:
    static void Init();
    static void Begin(std::function<void(uint8_t num, WStype_t type, uint8_t * payload, size_t length)> eventHandle);
    static void Loop();
    static void BroadcastImage();

private:
};

#endif