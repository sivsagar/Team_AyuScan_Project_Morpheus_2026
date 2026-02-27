# AyuScan — Low-Cost Wearable Vital Monitoring System

## Overview

AyuScan is a low-cost, wearable, IoT-based vital monitoring system designed to continuously measure key physiological parameters in real time. The system integrates biomedical sensors with an ESP32 microcontroller to acquire, process, and transmit health data wirelessly to a cloud dashboard for remote monitoring and early detection of abnormal conditions.

The device measures heart rate using both ECG and photoplethysmography (PPG), blood oxygen saturation (SpO₂), body temperature, and estimates cardiac output. Data is processed locally using filtering and peak detection algorithms to ensure accuracy and reliability.

All components are housed in a custom 3D-printed TPU enclosure, providing flexibility, durability, and comfort for wearable use.

## 🚀 Key Features

- **Real-time ECG-based heart rate monitoring** using the BioAmp EXG Pill.
- **PPG-based heart rate and SpO₂ measurement** using the MAX30102/MAX30105 sensor.
- **Non-contact body temperature measurement** using the MLX90614 infrared sensor.
- **Estimated cardiac output calculation** and wireless cloud monitoring using ESP32 and Blynk IoT.
- **Automatic abnormal condition detection** with live data visualization on mobile and web dashboards.
- **Compact, wearable design** using a custom 3D-printed TPU enclosure.

## 🛠 Hardware Components

| Component | Description |
| :--- | :--- |
| **ESP32** | 38-pin DevKit (Dual-core, Wi-Fi/Bluetooth enabled) |
| **BioAmp EXG Pill** | Analog front-end for ECG/EMG/EOG/EEG |
| **MAX30102 / MAX30105** | Pulse Oximetry and Heart-Rate sensor |
| **MLX90614** | Contactless Infrared (IR) Digital Temperature Sensor |

## 🔌 Pin Configuration

### I2C Bus (MAX3010x & MLX90614)
*Multiple sensors share the same I2C pins.*

| Sensor Pin | ESP32 Pin |
| :--- | :--- |
| **SDA** | GPIO 21 |
| **SCL** | GPIO 22 |

### BioAmp EXG Pill
| Sensor Pin | ESP32 Pin |
| :--- | :--- |
| **OUT** | GPIO 34 (ADC) |
| **VCC** | 3.3V / 5V |
| **GND** | GND |

## 🏗 System Architecture

```mermaid
graph TD
    A["BioAmp EXG Pill (ECG)"] --> E["ESP32"]
    B["MAX30102 (PPG HR + SpO₂)"] --> E
    C["MLX90614 (Temperature)"] --> E
    E --> F["Signal Processing"]
    F --> G["WiFi Transmission"]
    G --> H["Blynk Cloud Dashboard"]
    H --> I["Mobile and Web Monitoring"]
```

## 📊 Measured Parameters

- **ECG Heart Rate** (BPM)
- **PPG Heart Rate** (BPM)
- **Blood Oxygen Saturation** (SpO₂ %)
- **Body Temperature** (°C)
- **Cardiac Output** (L/min, estimated)
- **ECG Signal waveform** (raw)

## 📦 Software Dependencies

Ensure you have the following libraries installed in your Arduino IDE:

1. **SparkFun MAX3010x Pulse and Proximity Sensor Library**
2. **Adafruit MLX90614 Library**
3. **Blynk Library** (for cloud integration)

## 📂 Project Structure

- `BioAmpEXGPill.ino`: Heart rate monitoring via ECG.
- `MAX30105SpO2.ino`: Integrated HR and SpO2 monitoring.
- `TempMLX90614.ino`: Body temperature sensing.
- `MAX_calibiration.cpp`: Utility for calibrating the MAX3010x sensor brightness and thresholds.

## 📄 Usage Instructions

1. **Setup Hardware:** Connect sensors according to the [Pin Configuration](#-pin-configuration).
2. **Calibration:** Use `MAX_calibiration.cpp` to find optimal LED power for the MAX3010x sensor.
3. **Firmware Upload:** Upload the desired `.ino` file to the ESP32.
4. **Monitoring:** View live data on the **Serial Monitor (115200 baud)** or via the **Blynk mobile/web app**.

## 💡 Applications & Innovation

- **Remote Patient Monitoring & Telemedicine**: Scalable architecture for health tracking.
- **Fitness & Research**: Accurate real-time data for workout tracking and biomedical prototyping.
- **Innovation**: A single low-cost wearable platform combining multiple bio-sensing modalities with advanced cloud integration.

---
*Created as part of Project Morpheus 2026.*