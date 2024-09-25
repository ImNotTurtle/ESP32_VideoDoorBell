#include "../include/WebServer_Module.hpp"
#include "../include/Camera_Module.hpp"
#include "../include/WebSocket_Module.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#ifndef WEB_SERVER_DEFAULT_PORT
#define WEB_SERVER_DEFAULT_PORT 80
#endif

static AsyncWebServer webServer(WEB_SERVER_DEFAULT_PORT);

// static const char indexHTML[] PROGMEM = R"(
// <html>
// <head><title>ESP32-CAM WebSocket Streaming</title></head>
// <body>
// <h1>ESP32-CAM WebSocket Streaming</h1>

// <!-- Nút bấm -->
// <button onclick='ringDoorBell()'>Ring bell</button>

// <img id="stream" src="" style="width: 600px; height: 600px;"/>


// <script>
//     var ws = new WebSocket('ws://' + window.location.hostname + ':81/');
    
//     // Gửi chuỗi log khi bấm nút
//     function ringDoorBell() {
//         ws.send('RingDoorBell');
//     }

//     ws.onmessage = function(event) {
//         var image = event.data;  // Lấy dữ liệu hình ảnh từ WebSocket
//         document.getElementById('stream').src = image;
//     };
// </script>
// </body>
// </html>
// )";
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
    var ws = new WebSocket('ws://' + window.location.hostname + ':81/');
    ws.binaryType = 'arraybuffer';  // Thiết lập kiểu dữ liệu nhị phân
    
    // Gửi chuỗi log khi bấm nút
    function ringDoorBell() {
        ws.send('RingDoorBell');
    }

    // Nhận dữ liệu hình ảnh từ WebSocket
    ws.onmessage = function(event) {
        if (event.data instanceof ArrayBuffer) {
            // Chuyển đổi ArrayBuffer thành Blob và tạo URL hình ảnh
            var blob = new Blob([event.data], { type: 'image/jpeg' });
            var imageUrl = URL.createObjectURL(blob);
            
            // Gán URL hình ảnh cho thẻ <img>
            document.getElementById('stream').src = imageUrl;
        }
    };
</script>
</body>
</html>
)";

/*
if (image.startsWith('data:image/jpeg;base64,')) {  // Kiểm tra chuỗi base64
            document.getElementById('stream').src = image;  // Cập nhật thẻ <img>
        }
*/
/*
#WebSocketEventHeader: message = RingDoorBell (triggered when bell button pressed)
*/

void WebServer_Module::Init()
{
    webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", indexHTML);
    });

    Serial.printf("WebServer_Module : Init with port: %d\n", WEB_SERVER_DEFAULT_PORT);
}

void WebServer_Module::Begin()
{
    webServer.begin();
    //start stream task
    xTaskCreate(
        [](void *pvParameters) {
            while(true) {
                WebSocket_Module::BroadcastImage();
                vTaskDelay(30); //10ms between frames
            }
            //clean up
            vTaskDelete(NULL);   
        }, "WebServer_Module:streamTask", 4096, NULL, 1, NULL);
}

void WebServer_Module::Handle(){

}