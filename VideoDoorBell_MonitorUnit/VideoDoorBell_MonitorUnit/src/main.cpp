#include <Arduino.h>
#include <ArduinoJson.h>
#include <driver/i2s.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <TFT_eSPI.h>
// #include <SPI.h>
#include <TJpg_Decoder.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsClient.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Cấu hình I2S
#define I2S_SAMPLE_RATE   (16000)        // Tần số mẫu (16kHz)
#define I2S_PIN_BCK       (26)           // Chân BCK
#define I2S_PIN_WS        (25)           // Chân WS
#define I2S_PIN_DOUT      (22)           // Chân DOUT

const char *ssid = "ESP32-CAM-AP";
const char *password = "12345678";
IPAddress APIP(192, 168, 4, 1); // IP of Access Point (ESP32 - CAM)
uint8_t webSocketPort = 81;
TFT_eSPI tft = TFT_eSPI();
WebSocketsClient webSocket;
bool m_wifiConnected = false;
uint8_t imageBuffer[4096 * 3] = {0};
size_t imageLength = 0;

void setupI2S() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),  // Chế độ Master và Transmit (TX)
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,         // Mẫu 16-bit
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,         // Âm thanh 2 kênh (stereo)
        .communication_format = I2S_COMM_FORMAT_I2S,          // Chuẩn I2S
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,             // Ngắt cấp 1
        .dma_buf_count = 8,                                   // Bộ đệm DMA
        .dma_buf_len = 64,                                    // Độ dài bộ đệm
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_PIN_BCK,
        .ws_io_num = I2S_PIN_WS,
        .data_out_num = I2S_PIN_DOUT,
        .data_in_num = -1   // Không nhận input
    };

    // Thiết lập I2S driver
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_set_clk(I2S_NUM_0, I2S_SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_STEREO);
}

bool tftOutput(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap);
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);
void test();
void streamTask(void *arg);

void setup()
{
  Serial.begin(9600);

  // Kết nối vào WiFi (AP của ESP32-CAM)
  WiFi.begin(ssid, password);

  // Đợi đến khi kết nối thành công
  Serial.print("Wifi_Module : Connecting to ");
  Serial.print(ssid);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }

  m_wifiConnected = true;
  Serial.println();
  Serial.println("Wifi_Module : Connected to WiFi. ");
  Serial.printf("\tIP STA: %s. \n", WiFi.localIP());

  tft.begin();
  tft.setRotation(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
  tft.setTextFont(4);
  TJpgDec.setJpgScale(1);
  TJpgDec.setCallback(tftOutput);

  webSocket.begin(APIP.toString(), webSocketPort, "/"); // 192.168.4.1:81/
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);

  xTaskCreate(streamTask, "WebServer_Module:streamTask", 2048, NULL, 1, NULL);
}

void loop()
{
  if(m_wifiConnected){
    //handle web socket only when the wifi connect success
    webSocket.loop();
  }
}

void streamTask(void *arg)
{
  // test(); // test TFT LCD before streaming video
  vTaskDelay(pdMS_TO_TICKS(2000));
  while (true)
  {
    TJpgDec.drawJpg(0, 0, (const uint8_t *)imageBuffer, imageLength);
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
  // clean up

  vTaskDelete(NULL);
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.println("WebSocket_Module : Disconnected!");
    break;
  case WStype_CONNECTED:
    Serial.println("WebSocket_Module : Connected to WebSocket server");
    break;
  case WStype_BIN:
    // copy the payload to buffer
    imageLength = length;
    for (size_t i = 0; i < length; i++)
    {
      imageBuffer[i] = payload[i];
    }
    break;
  default:
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
// example link: https://github.com/Bodmer/TFT_eSPI/blob/master/examples/160%20x%20128/TFT_Rainbow/TFT_Rainbow.ino
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