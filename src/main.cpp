#include <Arduino.h>
#include <ESP32Servo.h>
#include <esp_now.h>
#include <WiFi.h>

#define SERVO_PIN 13
#define PUSH_BUTTON 19
#define LDR_PIN 36

uint8_t Controller1Address[] = {0x3C, 0x61, 0x05, 0x03, 0xA2, 0x74};
esp_now_peer_info_t peerInfoController1;


typedef struct MoleMessage {
  int mole;
} MoleMessage;



MoleMessage MoleInfo;
Servo myServo;




void UpMole() {
  Serial.println("UP MOLE");
  myServo.write(90);
}

void DownMole() {
  Serial.println("DOWN MOLE");
  myServo.write(0);
}


bool Debounce() {
  if (digitalRead(PUSH_BUTTON) == LOW) {
    delay(50);
    if (digitalRead(PUSH_BUTTON) == HIGH) {
      return true;
    }
  } 
  return false;

}



void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");

}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  // memcpy(&target_message, incomingData, sizeof(target_message));
  // if (target_message.targetState == 0) {
  //   downTarget();
  // } else {
  //   upTarget();
  // }
  memcpy(&MoleInfo, incomingData, sizeof(MoleInfo));
  if (MoleInfo.mole == 1) {
    UpMole();
  }
  
}


void setup() {

  // setup BUTTON
  pinMode(PUSH_BUTTON, INPUT_PULLUP);

  Serial.begin(9600); 
  WiFi.mode(WIFI_STA);

  // init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initilizing ESP-NOW");
    return;
  }

  // Do OnDataSent when send
  esp_now_register_send_cb(OnDataSent);

  // Do OnDataRecv when recieve
  esp_now_register_recv_cb(OnDataRecv);

  // register peer Controller1
  memcpy(peerInfoController1.peer_addr, Controller1Address, 6);
  peerInfoController1.channel = 0;
  peerInfoController1.encrypt = false;
  if (esp_now_add_peer(&peerInfoController1) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  
  //setup SERVO
  myServo.attach(SERVO_PIN);
  myServo.write(0);
}





void loop() {
  if (Debounce() && MoleInfo.mole) { // the botton is pressed
    // if MOLE UP do this 
    esp_err_t result = esp_now_send(Controller1Address, (uint8_t *) &MoleInfo, sizeof(MoleInfo)); 
    MoleInfo.mole = 0;
  }  
  
  if (MoleInfo.mole == 0) {
    DownMole();
  }

}
