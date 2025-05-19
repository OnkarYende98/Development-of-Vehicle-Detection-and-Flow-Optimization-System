# Smart Vehicle Detection and Flow Optimizer

## Overview

The **Smart Vehicle Detection and Flow Optimizer** is an embedded system designed to monitor and optimize vehicle traffic flow using real-time object detection powered by an ESP32-CAM module and Edge Impulse's machine learning capabilities. This project aims to provide intelligent traffic management by detecting vehicles, counting their numbers, and optimizing flow to reduce congestion and improve urban mobility.

## Features

1) Real-time vehicle detection using ESP32-CAM with onboard camera.
2) Integration with Edge Impulse’s FOMO object detection model for efficient and accurate detection.
3) Data display on an OLED screen (SSD1306) showing detected vehicle count and processing metrics.
4) Configurable detection time window to aggregate vehicle counts and flow statistics.
5) Optimized for low power consumption and cost-effective hardware deployment.
6) Suitable for smart city applications, parking management, and traffic analytics.

## Hardware Components

1) ESP32-CAM Module (AI Thinker model):** Core processing unit with an integrated camera for image capture.
2) SSD1306 OLED Display:** For real-time display of detection results and system status.
3) I2C Interface: Communication protocol for the OLED display.
4) Power Supply: 5V regulated power source to power the ESP32-CAM and display.

## Circuitry and Connections

1) ESP32-CAM Pins:

  * Power: 5V and GND connected to stable power source.
  * Camera Pins: Configured as per AI Thinker specification.
  * I2C Pins: GPIO 15 (SDA), GPIO 14 (SCL) connected to the SSD1306 OLED.
    
2) OLED Display:

  * VCC connected to 3.3V or 5V (depending on module specs).
  * GND connected to common ground.
  * SDA and SCL connected to ESP32 I2C pins (GPIO15 and GPIO14).

## Software Architecture

1) Camera Initialization: Configures ESP32-CAM for image capture at QVGA resolution.
2) Image Capture: Captures frames periodically for analysis.
3) Edge Impulse Inference: Runs the FOMO object detection model to identify vehicles in captured frames.
4) Result Processing: Aggregates detected vehicle data over a 15-second window.
5) isplay Update: Outputs detection counts and processing time on the OLED display.
6) Resource Management: Manages memory allocation for frame buffers and handles camera deinitialization on completion.

## Installation and Usage

1. Clone the repository:

   git clone https://github.com/OnkarYende98/Smart-Vehicle-Detection-and-Flow-Optimizer.git
   cd Smart-Vehicle-Detection-and-Flow-Optimizer
   
2. Set up the development environment:

   * Install [Arduino IDE](https://www.arduino.cc/en/software) 
   * Install ESP32 board support and necessary libraries for OLED Display.
     
3. Configure the project:

   * Replace `xxxx_inferencing.h` with your Edge Impulse generated header file for the model.
   * Adjust any pin definitions if using different hardware versions.
   * View vehicle detection data on the OLED display.

## Future Enhancements

1) Integration with cloud platforms for remote monitoring.
2) Support for multiple object classes (e.g., cars, bikes, trucks).
3) Advanced analytics for traffic pattern prediction with YOLO Models.



