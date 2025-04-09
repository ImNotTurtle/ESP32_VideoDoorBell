# Video Doorbell - Smart Visual Doorbell System

## Introduction

This project is a smart video doorbell system using the **ESP32-CAM** and **TFT LCD display** to show real-time images of visitors. The system transmits images over Wi-Fi and supports two versions: a **Web Server** version and an optimized **WebSocket** version for better performance.

## Key Features

- Display visitor's image on the TFT LCD screen
- Send images to a Web Server or via WebSocket
- Control and view interface through a web browser
- Optionally record images or videos (extendable)
- Communication between ESP32-CAM and ESP32-WROOM-32D

## System Architecture

- **ESP32-CAM**: Captures images/video
- **ESP32-WROOM-32D**: Receives and displays images on the TFT LCD
- **TFT LCD Display**: Displays real-time visitor images
- **Wi-Fi**: Wireless data transmission between devices

```plaintext
[ESP32-CAM] ---> (WebServer/WebSocket) ---> [ESP32-WROOM + TFT LCD]
```
# Two Versions Available:
1. VideoDoorBell_CameraUnit: Web Server version (allows direct viewing through a browser)
2. VideoDoorBell_CameraUnit2: WebSocket version (improved performance, allows devices like PCs to access the camera stream without requiring the camera to host a web server, resulting in higher FPS and smoother image display)

# Technologies Used
1. ESP32-CAM
2. ESP32-WROOM-32D
3. TFT LCD (ST7735 / ILI9341 or similar)
4. Web Server / WebSocket
5. Arduino IDE / PlatformIO

# Installation Guide
## Hardware Requirements
1. 1x ESP32-CAM
2. 1x ESP32-WROOM-32D
3. 1x TFT LCD
4. Breadboard, jumper wires, push button
5. 8Ω speaker

# Flashing the Program
1. Clone the repository: git clone https://github.com/ImNotTurtle/ESP32_VideoDoorBell.git
2. Open with PlatformIO or Arduino IDE
3. Select the correct board and COM port
4. Update the Wi-Fi configuration in src/Wifi_Module.cpp
5. Choose the version you want to run:
    - 5.1 VideoDoorBell_CameraUnit for Web Server
    - 5.2 VideoDoorBell_CameraUnit2 for WebSocket
6. Upload the code to both the ESP32-CAM and ESP32-WROOM
7. After uploading and wiring according to the documentation, the ESP32-WROOM will show a rainbow screen to confirm the display is properly configured
8. If you're using the Web Server version, access http://192.168.1.50 from a browser on the same Wi-Fi network to view the camera stream
9. To access the camera remotely (outside your Wi-Fi network), configure Port Forwarding on your router (forward port 80 to IP 192.168.1.50). Then visit your_public_ip:80 to access the stream (find your public IP at whatismyip.com)
10. If you're using the WebSocket version, you can connect from any programming language or system to ws://192.168.1.50:81 to exchange data with the camera (e.g., receive images or send bell triggers)

## Future Development
This project has great potential to become a practical and essential smart home tool, especially for families focused on home security.
Future improvements may include:
1. Two-way audio (speaker + microphone)
2. Voiice interaction
3. AI integration for automatic visitor recognition

# Documentation
You can find all necessary resources in the Documentations folder, which should contain everything you need to rebuild or understand this project.
One notice is that some document files have .drawio extension, you should go to draw.io website and open the file on it
