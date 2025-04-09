#include "../include/WebServer_Module.hpp"
#include "../include/Camera_Module.hpp"
#include "../include/WebSocket_Module.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#ifndef WEB_SERVER_DEFAULT_PORT
#define WEB_SERVER_DEFAULT_PORT 80
#endif

static AsyncWebServer webServer(WEB_SERVER_DEFAULT_PORT);

static const char indexHTML[] PROGMEM = R"(
<html>
<head><title>ESP32-CAM WebSocket Streaming</title></head>
<body>
<h1>ESP32-CAM WebSocket Streaming</h1>

<!-- Nút bấm -->
<button onclick='ringDoorBell()'>Ring bell</button>

<img id="stream" src="" style="width: 600px; height: 600px;"/>

<script>
    // Kết nối WebSocket
    var ws = new WebSocket('ws://' + window.location.hostname + ':{PORT}/');
    ws.binaryType = 'arraybuffer';
    
    function ringDoorBell() {
        ws.send('RingDoorBell');
    }

    ws.onmessage = function(event) {
        if (event.data instanceof ArrayBuffer) {
            var blob = new Blob([event.data], { type: 'image/jpeg' });
            var imageUrl = URL.createObjectURL(blob);
            document.getElementById('stream').src = imageUrl;
        }
    };
</script>
</body>
</html>
)";

void WebServer_Module::Init()
{
    //assign the web socket port
    String html = indexHTML;
    html.replace("{PORT}", String(WEB_SOCKET_DEFAULT_PORT));

    webServer.on("/", HTTP_GET, [=](AsyncWebServerRequest *request) {
        request->send(200, "text/html", html);
    });

    Serial.printf("WebServer_Module : Init with port: %d\n", WEB_SERVER_DEFAULT_PORT);


}

void WebServer_Module::Begin()
{
    webServer.begin();
}
