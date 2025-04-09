#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <TJpg_Decoder.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsClient.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "XT_DAC_Audio.hpp"
#include "sound.h"

#define DAC_1_PIN 25

const char *ssid = "ESP32-CAM-AP";
const char *password = "12345678";
const uint8_t webSocketPort = 81;
TFT_eSPI tft = TFT_eSPI();
WebSocketsClient webSocket;
bool m_wifiConnected = false;
uint8_t imageBuffer[4096 * 8] = {0};
size_t imageLength = 0;
unsigned long lastReceiveTime = 0;
const unsigned long receiveInterval = 1000; // 1000ms = 1 giây, tức là 1FPS

SemaphoreHandle_t frameReadySemaphore = xSemaphoreCreateBinary();
; // Cờ đánh dấu có khung hình mới để xử lý
XT_DAC_Audio_Class audioInstance = XT_DAC_Audio_Class(DAC_1_PIN, 0);
XT_Wav_Class doorBellSound = XT_Wav_Class(sound);
SemaphoreHandle_t imageMutex = xSemaphoreCreateMutex();

bool tftOutput(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap);
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);
void test();
void streamTask(void *arg);
void connectToWifi();

void setup()
{
    Serial.begin(9600);

    // Kết nối vào WiFi (AP của ESP32-CAM)
    WiFi.begin(ssid, password);

    // Đợi đến khi kết nối thành công
    connectToWifi();

    Serial.println();
    Serial.println("Wifi_Module : Connected to WiFi. ");
    Serial.printf("\tIP STA: %s. \n", WiFi.localIP().toString());
    Serial.printf("\tRSSI: %d", WiFi.RSSI());

    tft.begin();
    tft.setRotation(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    tft.setSwapBytes(true);
    tft.setTextFont(4);
    TJpgDec.setJpgScale(1);
    TJpgDec.setCallback(tftOutput);

    String gatewayIP = WiFi.gatewayIP().toString();
    webSocket.begin(gatewayIP.c_str(), webSocketPort, "/");

    webSocket.onEvent(webSocketEvent);
    webSocket.enableHeartbeat(15000, 3000, 2);
    webSocket.setReconnectInterval(5000);

    xTaskCreate(streamTask, "WebServer_Module:streamTask", 2048 * 2, NULL, 1, NULL);
}

void loop()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        // handle web socket only when the wifi connect success
        webSocket.loop();
        delay(10);
    }
    else
    {
        connectToWifi();
    }

    audioInstance.FillBuffer();
}

void connectToWifi()
{
    Serial.print("Wifi_Module : Connecting to ");
    Serial.print(ssid);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }
}

bool isJpegHeader(const uint8_t *data)
{
    return data[0] == 0xFF && data[1] == 0xD8; // JPEG header luôn bắt đầu với 0xFF 0xD8
}
void streamTask(void *arg)
{
    test(); // test TFT LCD to check if the wiring is correct before streaming video
    vTaskDelay(pdMS_TO_TICKS(10005));
    while (true)
    {
        // Chỉ vẽ khi có khung hình mới sẵn sàng
        if (xSemaphoreTake(frameReadySemaphore, portMAX_DELAY) == pdTRUE)
        {
            if (xSemaphoreTake(imageMutex, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                bool success = TJpgDec.drawJpg(0, 0, imageBuffer, imageLength);
                if (!success)
                {
                    Serial.println("Failed to decode JPEG image");
                }
                xSemaphoreGive(imageMutex);
            }
        }

        // Delay nhỏ để tránh chiếm CPU quá nhiều
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    // clean up
    vTaskDelete(NULL);
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
    {
        Serial.println("WebSocket: Disconnected!");
        // Serial.printf("Last WebSocket Error: %d\n", webSocket.());
        break;
    }
    case WStype_CONNECTED:
    {
        Serial.println("WebSocket: Connected to WebSocket server");
        break;
    }

    case WStype_BIN:
    {
        if (payload == NULL)
        {
            Serial.println("NULL payload");
            return;
        }

        // Kiểm tra xem đã đủ thời gian để nhận khung hình mới chưa
        unsigned long currentTime = millis();
        if (currentTime - lastReceiveTime >= receiveInterval)
        {
            // Chỉ xử lý nếu là JPEG hợp lệ
            if (length > 2 && isJpegHeader(payload))
            {
                if (xSemaphoreTake(imageMutex, pdMS_TO_TICKS(100)) == pdTRUE)
                {
                    imageLength = length < sizeof(imageBuffer) ? length : sizeof(imageBuffer);
                    memcpy(imageBuffer, payload, imageLength);

                    xSemaphoreGive(frameReadySemaphore); // Đánh dấu có khung hình mới
                    xSemaphoreGive(imageMutex);
                    Serial.printf("Received new frame at %lu ms, size: %d\n", currentTime, length);
                }
                // Cập nhật thời gian nhận khung hình cuối cùng
                lastReceiveTime = currentTime;
            }
            else if (length > 2)
            {
                Serial.println("Received data is not a valid JPEG image");
            }
        }
        // Nếu chưa đến thời gian nhận khung hình mới, bỏ qua khung hình này
        break;
    }

    case WStype_TEXT:
    {
        if (length == 8 && payload != NULL && memcmp(payload, "RingBell", 8) == 0)
        {
            // play door bell sound
            //   if(audioInstance.AlreadyPlaying(&doorBellSound) == false){
            audioInstance.StopAllSounds();
            audioInstance.Play(&doorBellSound);
            //   }
            Serial.println("Dingdonggg");
        }
        else
        {
            Serial.printf("Websocket: receive text with length: %d\n", length);
        }
        break;
    }
    default:
        Serial.printf("Websocket: Received type: %d, \n", type); // payload: %s, length: %d, (char*)payload, length);
        break;
    }
}

bool tftOutput(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    if (y >= tft.height())
        return 0;
    tft.pushImage(x, y, w, h, bitmap);
    return 1;
}

// test function to test for LCD is working or not
// reference source: https://github.com/Bodmer/TFT_eSPI/blob/master/examples/160%20x%20128/TFT_Rainbow/TFT_Rainbow.ino
unsigned long targetTime = 0;
byte red = 31;
byte green = 0;
byte blue = 0;
byte state = 0;
unsigned int colour = red << 11;
void test()
{
    targetTime = millis() + 10000;

    // Colour changing state machine
    for (int i = 0; i < 160; i++)
    {
        tft.drawFastVLine(i, 0, tft.height(), colour);
        switch (state)
        {
        case 0:
            green += 2;
            if (green == 64)
            {
                green = 63;
                state = 1;
            }
            break;
        case 1:
            red--;
            if (red == 255)
            {
                red = 0;
                state = 2;
            }
            break;
        case 2:
            blue++;
            if (blue == 32)
            {
                blue = 31;
                state = 3;
            }
            break;
        case 3:
            green -= 2;
            if (green == 255)
            {
                green = 0;
                state = 4;
            }
            break;
        case 4:
            red++;
            if (red == 32)
            {
                red = 31;
                state = 5;
            }
            break;
        case 5:
            blue--;
            if (blue == 255)
            {
                blue = 0;
                state = 0;
            }
            break;
        }
        colour = red << 11 | green << 5 | blue;
    }

    // The standard ADAFruit font still works as before
    tft.setTextColor(TFT_BLACK);
    tft.setCursor(12, 5);
    tft.print("Original ADAfruit font!");

    // The new larger fonts do not use the .setCursor call, coords are embedded
    tft.setTextColor(TFT_BLACK, TFT_BLACK); // Do not plot the background colour

    // Overlay the black text on top of the rainbow plot (the advantage of not drawing the background colour!)
    tft.drawCentreString("Font size 2", 80, 14, 2); // Draw text centre at position 80, 12 using font 2

    // tft.drawCentreString("Font size 2",81,12,2); // Draw text centre at position 80, 12 using font 2

    tft.drawCentreString("Font size 4", 80, 30, 4); // Draw text centre at position 80, 24 using font 4

    tft.drawCentreString("12.34", 80, 54, 6); // Draw text centre at position 80, 24 using font 6

    tft.drawCentreString("12.34 is in font size 6", 80, 92, 2); // Draw text centre at position 80, 90 using font 2

    // Note the x position is the top left of the font!

    // draw a floating point number
    float pi = 3.14159;                                     // Value to print
    int precision = 3;                                      // Number of digits after decimal point
    int xpos = 50;                                          // x position
    int ypos = 110;                                         // y position
    int font = 2;                                           // font number only 2,4,6,7 valid. Font 6 only contains characters [space] 0 1 2 3 4 5 6 7 8 9 0 : a p m
    xpos += tft.drawFloat(pi, precision, xpos, ypos, font); // Draw rounded number and return new xpos delta for next print position
    tft.drawString(" is pi", xpos, ypos, font);             // Continue printing from new x position
}
