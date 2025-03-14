#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>

#define LED_PIN 3

// this esp's mac: 
// 64:e8:33:b5:e9:98

Servo s;
float result = 0;
float percents = 0;

void onDataReceived(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
    int command;
    int potentiometer = command;
    memcpy(&command, incomingData, sizeof(command));
    potentiometer = command;
    percents = (100.0 / 4096.0) * ((float)command);
    result = (int)((180.0 / 100.0) * (int)percents);
    result = potentiometer * 4 / 4095 * 45;

    Serial.print("Data received: ");
    Serial.print(command);
    Serial.print(" result is ");
    Serial.print(result);
    Serial.print(" percents is ");
    Serial.println(percents);

    s.write(result);
}

void setup() {
    Serial.begin(19200);
    WiFi.mode(WIFI_STA); 

    pinMode(LED_PIN, OUTPUT);
    s.attach(LED_PIN);

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW Init Failed");
        return;
    }
    esp_now_register_recv_cb(onDataReceived);
}

void loop() {
  
}
