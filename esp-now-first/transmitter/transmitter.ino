#include <esp_now.h>
#include <WiFi.h>

#define BUTTON_PIN 3
#define POTENT_PIN 2

uint8_t receiverAddress[] = {0x64, 0xE8, 0x33, 0xB5, 0xE9, 0x98}; 

void sendData() {
    int command = digitalRead(BUTTON_PIN) == LOW ? 1 : 0;
    int pontetiometerValue = analogRead(POTENT_PIN);
    esp_now_send(receiverAddress, (uint8_t *)&pontetiometerValue, sizeof(pontetiometerValue));

    Serial.print("Data sent: ");
    Serial.println(command);
}

void setup() {
    Serial.begin(9600);
    WiFi.mode(WIFI_STA);

    pinMode(BUTTON_PIN, INPUT_PULLUP); 
    
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW Init Failed");
        return;
    }
    
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, receiverAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);
}

void loop() {
    sendData();  
    delay(100); // Send every 100ms
}
