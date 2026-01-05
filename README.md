# ESP32 Wi-Fi LED Control Web Server

## Overview
This project implements an ESP32-based embedded system that hosts a local HTTP web server, allowing users to control LED states remotely over Wi-Fi. The system demonstrates hardware–software integration, networking, and real-time GPIO control.

## Features
- ESP32-hosted HTTP web server
- LED control via browser-based interface
- GPIO output control triggered by HTTP requests
- Real-time response over local Wi-Fi network

## Technologies Used
- ESP32
- Embedded C / C++
- HTTP
- Wi-Fi
- GPIO

## How It Works
1. ESP32 connects to a local Wi-Fi network
2. The device hosts a simple HTTP server
3. A browser sends HTTP requests when buttons are pressed
4. Requests are parsed by the firmware
5. Corresponding GPIO pins are toggled to control LEDs

## Getting Started
### Hardware
- ESP32 development board
- LEDs
- Current-limiting resistors
- Breadboard and jumper wires

### Software
- ESP-IDF or Arduino framework
- USB connection for flashing

## Future Improvements
- Implement authentication
- Add more LEDs
- Improve UI styling
