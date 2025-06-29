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

### 🔩 Components

- **ESP32 Devkit V1** – Central processing unit.
- **ADAFRUIT SSD1306 OLED Monochrome Display** – Displays current time, medication reminders, and alerts.
- **DHT11 Temperature and Humidity Sensor** – Measures environmental conditions.
- **Breadboard Setup** – All components are prototyped on a breadboard.

---

### 🛠️ Development Setup

- **PlatformIO with Arduino Framework** – Used for programming the ESP32.
- **Wokwi Simulation** – Simulate the functionality before the physical implementation.

---

### 🚀 Usage

1. Configure the ESP32 with your network to connect to the NTP server.
2. Set up medication reminders and environmental thresholds through the device menu.
3. Receive alerts and manage settings directly via the OLED display.

---
## 📸 Demo & Media

> ![Device Setup] ![image](https://github.com/user-attachments/assets/665347aa-9352-4441-90d6-ec289cd675a5)


## 🌟 Stage 2: Intelligent Light & Environment Control

This stage enhances the Smart Medibox with automated light regulation and advanced environmental control using light sensors and a servo-controlled shaded window.

---

### 🌞 Light Intensity Monitoring

- Utilizes an **LDR (Light Dependent Resistor)** to measure ambient light.
- **Sampling Interval (ts)**: Default 5 seconds (user-configurable).
- **Sending Interval (tu)**: Default 2 minutes (user-configurable).
- Averages light intensity readings and sends the data to the Node-RED dashboard.

---

### 📊 Node-RED Dashboard – Light Group

- **Numerical Display**: Shows latest average light intensity (range 0–1).
- **Chart**: Historical light intensity trends.
- **Two Sliders**:
  - Sampling interval (`ts`) in seconds
  - Sending interval (`tu`) in seconds or minutes

---

### 🤖 Shaded Sliding Window – Servo Control

A servo motor adjusts the window angle to regulate light exposure based on environmental conditions.

**Equation for Motor Angle**:


Where:

- `θ` = motor angle  
- `θoffset` = minimum angle (default 30°)  
- `I` = light intensity (0 to 1)  
- `γ` = controlling factor (default 0.75)  
- `ts` = sampling interval (seconds)  
- `tu` = sending interval (seconds)  
- `T` = measured temperature (°C)  
- `Tmed` = ideal medicine temperature (default 30°C)

---

### ⚙️ Node-RED Dashboard – Motor Control Group

Includes 3 sliders to customize control parameters:

- **Minimum Angle (θoffset)**: 0° to 120°
- **Controlling Factor (γ)**: 0.0 to 1.0
- **Ideal Storage Temperature (Tmed)**: 10°C to 40°C

---

## 🔩 Full Component List

| Component                     | Description                                      |
|------------------------------|--------------------------------------------------|
| ESP32 Devkit V1              | Microcontroller for processing and control       |
| SSD1306 OLED Display         | User interface for reminders and alerts          |
| DHT11                        | Measures internal temperature and humidity       |
| LDR + Resistor               | Reads ambient light intensity                    |
| Servo Motor                  | Controls shaded window to regulate lighting      |
| Breadboard + Jumpers         | Prototyping setup                                |
| Node-RED Dashboard           | Web interface for configuration and visualization|

---

## 📊 Dashboard Overview

> *(Replace with your own screenshots)*  


## 📸 Demo & Media

> ![Device Setup] ![image](https://github.com/user-attachments/assets/e12bbdf4-674d-4b08-b1b8-d304eecc83c4)



