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
}

void WebSocket_Module::Loop()
{
    webSocket.loop();
}

void WebSocket_Module::BroadcastImage()
{
    Camera_Module::CaptureImage([](uint8_t* buffer, size_t len) { // capture image success
        // String imageBase64 = "data:image/jpeg;base64,";
        // // imageBase64 += base64::encode(fb->buf, fb->len);  // Encode to base64
        // // webSocket.broadcastTXT(imageBase64);
        // int inputStringLength = imageBase64.length();
        // char inputString[inputStringLength];
        // strncpy(inputString, imageBase64.c_str(), inputStringLength);

        // int encodedLength = Base64.encodedLength(inputStringLength);
        // char encodedString[encodedLength];
        // Base64.encode(encodedString, inputString, inputStringLength);

        // // // Send the base64 image via WebSocket
        // webSocket.broadcastTXT(encodedString);
        webSocket.broadcastBIN(buffer, len);
    });
}