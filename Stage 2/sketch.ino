#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define BUZZER 16
#define LED_1 15
#define PB_CANCEL 34
#define PB_OK 35
#define PB_UP 33
#define PB_DOWN 32
#define DHTPIN 14
#define LDR_PIN 36 // VP pin for LDR
#define SERVO_PIN 18

#define NTP_SERVER "pool.ntp.org"
#define MQTT_SERVER "broker.hivemq.com"
#define MQTT_PORT 1883

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHTesp dhtSensor;
WiFiClient espClient;
PubSubClient client(espClient);
Servo servo;

// Global variables for existing functionality
int days = 0, hours = 0, minutes = 0, seconds = 0;
unsigned long timeNow = 0, timeLast = 0;
bool alarm_enabled = true;
int n_alarms = 3;
int alarm_hours[] = {0, 1, 2};
int alarm_minutes[] = {1, 10, 20};
bool alarm_triggered[] = {false, false, false};
int n_notes = 8;
int notes[] = {262, 294, 330, 349, 392, 440, 494, 523};
int current_mode = 0;
int max_modes = 5;
String modes[] = {"1 - Set Time Zone", "2 - Set Alarm 1", "3 - Set Alarm 2", "4 - Set Alarm 3", "5 - Disable Alarms"};

// Variables for LDR and servo
float ts = 5.0; // Sampling interval in seconds
float tu = 120.0; // Sending interval in seconds
float theta_offset = 30.0; // Minimum servo angle
float gamma_val = 0.75; // Controlling factor
float T_med = 30.0; // Ideal temperature
float light_readings[60]; // Store up to 60 readings (300s / 5s)
int reading_count = 0;
unsigned long last_sample_time = 0;
unsigned long last_send_time = 0;
float current_temp = 25.0;  // Current Temperature from DHT22 Sensor
float current_intensity = 0.5;

void setup() {
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(PB_CANCEL, INPUT);
  pinMode(PB_OK, INPUT);
  pinMode(PB_UP, INPUT);
  pinMode(PB_DOWN, INPUT);
  pinMode(LDR_PIN, INPUT);

  dhtSensor.setup(DHTPIN, DHTesp::DHT22);
  servo.setPeriodHertz(50); // 50 Hz for SG90
  servo.attach(SERVO_PIN, 500, 2400); // Min 500µs, max 2400µs

  Serial.begin(9600);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    display.clearDisplay();
    print_line("Connecting to WIFI", 0, 0, 2);
  }
  display.clearDisplay();
  print_line("Connected to WIFI", 0, 0, 2);

  configTime(0, 0, NTP_SERVER);
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(mqttCallback);
  reconnectMQTT();

  display.display();
  delay(2000);
  display.clearDisplay();
  print_line("Welcome to Medibox!", 10, 20, 2);
  display.clearDisplay();
}

void loop() {
  if (!client.connected()) reconnectMQTT();
  client.loop();

  update_time_with_check_alarm();
  if (digitalRead(PB_OK) == LOW) {
    delay(200);
    go_to_menu();
  }
  check_temp();
  handleLDR();
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("medibox/220193M/ts");
      client.subscribe("medibox/220193M/tu");
      client.subscribe("medibox/220193M/offset");
      client.subscribe("medibox/220193M/gamma_val");
      client.subscribe("medibox/220193M/temp");
    } else {
      Serial.print("[MQTT] Connection failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("MQTT Message Received - Topic: ");
  Serial.print(topic);
  Serial.print(" | Message: ");
  Serial.println(message);

  if (String(topic) == "medibox/220193M/ts") {
    ts = message.toFloat();
    if (ts < 1) ts = 1;
    Serial.println("[Updated] Sampling Interval (ts): " + String(ts));
    reading_count = 0;
    updateServo(current_intensity, current_temp, theta_offset, gamma_val, ts, tu, T_med);
  } else if (String(topic) == "medibox/220193M/tu") {
    tu = message.toFloat();
    if (tu < 10) tu = 10;
    Serial.println("[Updated] Sending Interval (tu): " + String(tu));
    reading_count = 0;
    updateServo(current_intensity, current_temp, theta_offset, gamma_val, ts, tu, T_med);
  } else if (String(topic) == "medibox/220193M/offset") {
    theta_offset = message.toFloat();
    if (theta_offset < 0) theta_offset = 0;
    if (theta_offset > 120) theta_offset = 120;
    Serial.println("[Updated] Minimum Servo Angle (theta_offset): " + String(theta_offset));
    updateServo(current_intensity, current_temp, theta_offset, gamma_val, ts, tu, T_med);
  } else if (String(topic) == "medibox/220193M/gamma_val") {
    gamma_val = message.toFloat();
    if (gamma_val < 0) gamma_val = 0;
    if (gamma_val > 1) gamma_val = 1;
    Serial.println("[Updated] Controlling Factor (gamma_val): " + String(gamma_val));
    updateServo(current_intensity, current_temp, theta_offset, gamma_val, ts, tu, T_med);
  } else if (String(topic) == "medibox/220193M/temp") {
    T_med = message.toFloat();
    if (T_med < 10) T_med = 10;
    if (T_med > 40) T_med = 40;
    Serial.println("[Updated] Ideal Storage Temperature (T_med): " + String(T_med));
    updateServo(current_intensity, current_temp, theta_offset, gamma_val, ts, tu, T_med);
  }
}

void handleLDR() {
  unsigned long current_time = millis();
  
  // Sample LDR at ts intervals
  if (current_time - last_sample_time >= ts * 1000) {
    float ldr_raw = analogRead(LDR_PIN);
    float light_intensity = ldr_raw / 4095.0; // Normalize to 0-1
    current_intensity = light_intensity;
    
    // Publish current light intensity
    char buffer[10];
    dtostrf(light_intensity, 6, 2, buffer); // Increased width for safety
    if (client.publish("medibox/220193M/lightIntensity", buffer)) {
      Serial.println("[MQTT] Published lightIntensity: " + String(buffer));
    } else {
      Serial.println("[MQTT] Failed to publish lightIntensity");
    }
    
    light_readings[reading_count] = light_intensity;
    reading_count++;
    last_sample_time = current_time;
    Serial.println("[LDR Sampled] Raw: " + String(ldr_raw) + " | Normalized: " + String(light_intensity));

    // Calculate max readings based on tu and ts
    int max_readings = (int)(tu / ts);
    if (max_readings < 1) max_readings = 1; // Prevent division by zero
    
    Serial.println("[LDR] reading_count: " + String(reading_count) + ", max_readings: " + String(max_readings) + ", time_elapsed: " + String(current_time - last_send_time) + "ms");

    // Check if it's time to send average intensity
    if (reading_count >= max_readings || current_time - last_send_time >= tu * 1000) {
      if (reading_count > 0) { // Ensure there are readings to average
        float sum = 0;
        for (int i = 0; i < reading_count; i++) {
          sum += light_readings[i];
        }
        float avg_intensity = sum / reading_count;
        
        // Publish average intensity
        dtostrf(avg_intensity, 6, 2, buffer); // Increased width for safety
        if (client.publish("medibox/220193M/avg_intensity", buffer)) {
          Serial.println("[MQTT] Published avg_intensity: " + String(buffer));
        } else {
          Serial.println("[MQTT] Failed to publish avg_intensity");
        }
        
        Serial.println("[LDR Average Sent] Avg Intensity: " + String(avg_intensity));
        
        // Update servo with average intensity
        updateServo(avg_intensity, current_temp, theta_offset, gamma_val, ts, tu, T_med);
        
        reading_count = 0; // Reset reading count
        last_send_time = current_time; // Update last send time
      } else {
        Serial.println("[LDR] No readings to average");
      }
    } else {
      // Update servo with current intensity if not averaging
      updateServo(current_intensity, current_temp, theta_offset, gamma_val, ts, tu, T_med);
    }
  }
}

void updateServo(float avg_intensity, float T, float theta_offset, float gamma_val, float ts, float tu, float T_med) {
  float theta = theta_offset + (180 - theta_offset) * avg_intensity * gamma_val * log(ts / tu) * (T / T_med);
  theta = constrain(theta, 0, 180);
  Serial.println("[Servo Control] Final Angle θ = " + String(theta));

  servo.write((int)theta);
}

void print_line(String text, int column, int row, int text_size) {
  display.setTextSize(text_size);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(column, row);
  display.println(text);
  display.display();
}

void print_time_now(void) {
  display.clearDisplay();
  print_line(String(days), 0, 0, 2);
  print_line(":", 20, 0, 2);
  print_line(String(hours), 30, 0, 2);
  print_line(":", 50, 0, 2);
  print_line(String(minutes), 60, 0, 2);
  print_line(":", 80, 0, 2);
  print_line(String(seconds), 90, 0, 2);
}

void update_time(void) {
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  char day_str[8], hour_str[8], min_str[8], sec_str[8];
  strftime(day_str, 8, "%d", &timeinfo);
  strftime(sec_str, 8, "%S", &timeinfo);
  strftime(hour_str, 8, "%H", &timeinfo);
  strftime(min_str, 8, "%M", &timeinfo);
  hours = atoi(hour_str);
  minutes = atoi(min_str);
  days = atoi(day_str);
  seconds = atoi(sec_str);
}

void update_time_with_check_alarm() {
  update_time();
  print_time_now();
  if (alarm_enabled) {
    for (int i = 0; i < n_alarms; i++) {
      if (!alarm_triggered[i] && hours == alarm_hours[i] && minutes == alarm_minutes[i]) {
        ring_alarm();
        alarm_triggered[i] = true;
      }
    }
  }
}

void ring_alarm() {
  display.clearDisplay();
  print_line("Medicine Time", 0, 0, 2);
  bool break_happened = false;
  digitalWrite(LED_1, HIGH);
  while (!break_happened && digitalRead(PB_CANCEL) == HIGH) {
    for (int i = 0; i < n_notes; i++) {
      if (digitalRead(PB_CANCEL) == LOW) {
        delay(200);
        break_happened = true;
        break;
      }
      tone(BUZZER, notes[i]);
      delay(500);
      noTone(BUZZER);
      delay(2);
    }
  }
  digitalWrite(LED_1, LOW);
  display.clearDisplay();
}

int wait_for_button_press() {
  while (true) {
    if (digitalRead(PB_UP) == LOW) {
      delay(200);
      return PB_UP;
    } else if (digitalRead(PB_DOWN) == LOW) {
      delay(200);
      return PB_DOWN;
    } else if (digitalRead(PB_CANCEL) == LOW) {
      delay(200);
      return PB_CANCEL;
    } else if (digitalRead(PB_OK) == LOW) {
      delay(200);
      return PB_OK;
    }
    update_time();
  }
}

void go_to_menu(void) {
  while (digitalRead(PB_CANCEL) == HIGH) {
    display.clearDisplay();
    print_line(modes[current_mode], 0, 0, 2);
    int pressed = wait_for_button_press();
    if (pressed == PB_UP) {
      current_mode = (current_mode + 1) % max_modes;
      delay(200);
    } else if (pressed == PB_DOWN) {
      current_mode = (current_mode - 1 + max_modes) % max_modes;
      delay(200);
    } else if (pressed == PB_OK) {
      delay(200);
      run_mode(current_mode);
    } else if (pressed == PB_CANCEL) {
      delay(200);
      break;
    }
  }
}

void run_mode(int mode) {
  if (mode == 0) {
    set_time_zone();
  } else if (mode >= 1 && mode <= 3) {
    set_alarm(mode - 1);
  } else if (mode == 4) {
    alarm_enabled = false;
  }
}

void set_time_zone() {
  int temp_utc_offset = 0;
  display.clearDisplay();
  print_line("Set UTC offset (HH:MM):", 0, 0, 1);
  while (true) {
    int offset_hours = temp_utc_offset / 60;
    int offset_minutes = abs(temp_utc_offset % 60);
    display.setCursor(0, 16);
    display.printf("%+03d:%02d", offset_hours, offset_minutes);
    display.display();
    int pressed = wait_for_button_press();
    if (pressed == PB_UP) {
      temp_utc_offset += 30;
      delay(200);
    } else if (pressed == PB_DOWN) {
      temp_utc_offset -= 30;
      delay(200);
    } else if (pressed == PB_OK) {
      configTime(temp_utc_offset * 60, 0, NTP_SERVER);
      display.clearDisplay();
      print_line("Time zone set", 0, 16, 1);
      delay(1000);
      break;
    } else if (pressed == PB_CANCEL) {
      break;
    }
    display.clearDisplay();
    print_line("Set UTC offset (HH:MM):", 0, 0, 1);
  }
  display.clearDisplay();
}

void set_alarm(int alarm) {
  int temp_hour = alarm_hours[alarm];
  while (true) {
    display.clearDisplay();
    print_line("Enter hour: " + String(temp_hour), 0, 0, 2);
    int pressed = wait_for_button_press();
    if (pressed == PB_UP) {
      temp_hour = (temp_hour + 1) % 24;
      delay(200);
    } else if (pressed == PB_DOWN) {
      temp_hour = (temp_hour - 1 + 24) % 24;
      delay(200);
    } else if (pressed == PB_OK) {
      alarm_hours[alarm] = temp_hour;
      delay(200);
      break;
    } else if (pressed == PB_CANCEL) {
      delay(200);
      break;
    }
  }
  int temp_minute = alarm_minutes[alarm];
  while (true) {
    display.clearDisplay();
    print_line("Enter minute: " + String(temp_minute), 0, 0, 2);
    int pressed = wait_for_button_press();
    if (pressed == PB_UP) {
      temp_minute = (temp_minute + 1) % 60;
      delay(200);
    } else if (pressed == PB_DOWN) {
      temp_minute = (temp_minute - 1 + 60) % 60;
      delay(200);
    } else if (pressed == PB_OK) {
      alarm_minutes[alarm] = temp_minute;
      delay(200);
      break;
    } else if (pressed == PB_CANCEL) {
      delay(200);
      break;
    }
  }
  display.clearDisplay();
  print_line("Alarm is set", 0, 0, 2);
  delay(1000);
}

void check_temp(void) {
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  if (dhtSensor.getStatus() == DHTesp::ERROR_NONE) {
    current_temp = data.temperature;
    Serial.println("[Sensor] Temp = " + String(data.temperature) + " °C, Hum = " + String(data.humidity) + " %");
    
    char buffer[10];
    dtostrf(current_temp, 6, 1, buffer); // Increased width for safety
    if (client.publish("medibox/220193M/current_temp", buffer)) {
      Serial.println("[MQTT] Published current_temp: " + String(buffer));
    } else {
      Serial.println("[MQTT] Failed to publish current_temp");
    }
    
    updateServo(current_intensity, current_temp, theta_offset, gamma_val, ts, tu, T_med);
  } else {
    Serial.println("[Sensor] Failed to read from DHT22, status: " + String(dhtSensor.getStatusString()));
  }
}