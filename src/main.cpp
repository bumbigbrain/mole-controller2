#include <Arduino.h>
#include <ESP32Servo.h>
#include <esp_now.h>
#include <WiFi.h>

#define SERVO_PIN 13
#define PUSH_BUTTON 19
#define LDR_PIN 36

uint8_t stateControllerAddress[] = {0x3C, 0x61, 0x05, 0x03, 0xD4, 0xB0};

typedef struct targetMessage {
  int targetState;
} targetMessage; 

Servo myServo;
targetMessage target_message;
esp_now_peer_info_t peerInfo;




void upTarget() {
  Serial.printf("UP TARGET : ");
  myServo.write(90);
  Serial.println(WiFi.macAddress());
}

void downTarget() {
  Serial.printf("DOWN TARGET : ");
  myServo.write(0);
  Serial.println(WiFi.macAddress());
}


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");

}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&target_message, incomingData, sizeof(target_message));
  if (target_message.targetState == 0) {
    downTarget();
  } else {
    upTarget();
  }

  
}


void setup() {


  //Button Setting
  pinMode(PUSH_BUTTON, INPUT_PULLUP);

  //ESPNOW setup SENDING
  Serial.begin(9600); 
  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_send_cb(OnDataSent);

  //ESPNOW setup RECIEVING
  esp_now_register_recv_cb(OnDataRecv);

  // Register stateControllerPeer
  memcpy(peerInfo.peer_addr, stateControllerAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;


  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  
  //Servo 
  myServo.attach(SERVO_PIN);
}

void loop() {
  if (digitalRead(PUSH_BUTTON) == LOW) { // if target is shot
    esp_err_t result = esp_now_send(stateControllerAddress, (uint8_t *) &target_message, sizeof(target_message));
    downTarget();
    delay(500); 
  } 
  
  // LDR TESTING
  // int val = analogRead(LDR_PIN); 
  // Serial.print("ldr val = ");
  // Serial.println(val);
  // delay(500);

  
}
