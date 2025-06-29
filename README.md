# 💊 Smart Medibox

A smart IoT-based device designed to help users manage their medication schedules effectively while maintaining optimal storage conditions using real-time monitoring.

---

## 📸 Demo Preview

### 🖼️ Device Overview
![Smart Medibox Device Setup](docs/images/device-overview-placeholder.jpg)
*Image: Replace with actual photo of the full setup*

### 🎥 Live Demo
[![Watch the video](docs/images/video-thumbnail-placeholder.jpg)](https://www.youtube.com/watch?v=your-demo-link)
*Click to watch full demonstration*

---

## 🔧 Features

- ⏰ **Medication Reminders**  
  Automates medication schedules with alarms and notifications to ensure timely doses.

- 🌐 **Time Zone Configuration**  
  Set your UTC offset to sync time from an NTP server accurately.

- 🌡️ **Environmental Monitoring**  
  Measures temperature and humidity with alerts if levels are outside the defined thresholds.

- 🖥️ **OLED Display Interface**  
  Displays time, upcoming medications, and environmental alerts clearly on a 128x64 monochrome screen.

---

## 🔩 Hardware Components

| Component                     | Description                             |
|------------------------------|-----------------------------------------|
| ESP32 DevKit V1              | Main microcontroller (Wi-Fi enabled)    |
| DHT11 Sensor                 | Temperature & humidity monitoring       |
| SSD1306 OLED Display (I2C)   | Displays time, alerts, and UI           |
| Breadboard + Jumper Wires    | For prototyping connections             |

---

## 🛠️ Development Environment

- **Platform**: [PlatformIO](https://platformio.org/) with the Arduino Framework  
- **Simulation**: [Wokwi](https://wokwi.com/) (for virtual testing before deployment)  
- **Code Editor**: Visual Studio Code  
- **Libraries**:
  - `Adafruit_SSD1306`
  - `Adafruit_GFX`
  - `DHT sensor library`
  - `NTPClient`
  - `WiFi.h`

---

## 🚀 Getting Started

### 🔌 Hardware Setup
1. Connect ESP32 to the OLED display using I2C (SDA → GPIO21, SCL → GPIO22).
2. Connect DHT11 to ESP32 (e.g., DATA → GPIO15).
3. Power the ESP32 via USB.

### 💻 Software Setup
1. Clone this repository  
   ```bash
   git clone https://github.com/your-username/smart-medibox.git
   cd smart-medibox
