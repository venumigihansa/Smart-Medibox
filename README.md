# 💊 Smart Medibox

Smart Medibox is a device designed to aid in the effective management of medication by ensuring timely intakes and maintaining optimal storage conditions through sophisticated monitoring and control systems.

---

## 🚦 Stage 1: Basic Functionality

### ✅ Features

- **Medication Reminders**  
  Automates medication schedules with alarms to ensure doses are not missed.

- **Time Zone Settings**  
  Users can set their local time zone by specifying the UTC offset, which is then used to fetch the current time from the NTP server.

- **Environmental Monitoring**  
  Monitors temperature and humidity, alerting if conditions fall outside the healthy ranges.

---

## 🔩 Components

- **ESP32 Devkit V1**  
  Central processing unit.

- **ADAFRUIT SSD1306 OLED Monochrome Display**  
  Displays current time, medication reminders, and alerts.

- **DHT11 Temperature and Humidity Sensor**  
  Measures environmental conditions.

- **Breadboard Setup**  
  All components are mounted and wired on a breadboard for prototyping.

---

## 🛠️ Development Setup

- **PlatformIO with Arduino Framework**  
  Used for programming the ESP32.

- **Wokwi Simulation**  
  Simulate the functionality before the physical implementation.

---

## 🚀 Usage

1. **Connect to Wi-Fi**  
   Configure the ESP32 with your network to connect to the NTP server.

2. **Set Reminders**  
   Set up medication reminders and environmental thresholds through the device menu.

3. **Receive Alerts**  
   Receive alerts and manage settings directly via the OLED display.

---

## 📸 Demo & Media

> *(Add your images and videos here)*  
> ![Device Setup](docs/images/device-placeholder.jpg)  
