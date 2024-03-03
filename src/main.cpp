#include <Arduino.h>
#include <ESP32Servo.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SERVO_PIN 13
#define PUSH_BUTTON 19
#define LDR_PIN 36
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);





uint8_t Controller2Address[] = {0x3C, 0x61, 0x05, 0x03, 0xD4, 0xB0};
esp_now_peer_info_t peerInfoController2;


typedef struct MoleMessage {
  int mole;
} MoleMessage;




void displayNoMole() { 
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(23, 10);
  display.println("No Mole");
  display.display();
}


void displayMode0() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(15, 10);
  display.printf("GIGI MOLE");
  display.setCursor(10, 20);
  display.printf("1 HP");
  display.display();
}

void displayMode1() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(15, 10);
  display.printf("SUPER GIGI MOLE"); 
  display.setCursor(10, 20);
  display.printf("1 HP");
  display.setCursor(10, 30);
  display.printf("1 GUARD");
  display.display();
}

void displayMode2() { 
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(15, 20);
  display.printf("TANK MOLE");
  display.setCursor(10, 30);
  display.printf("3 HP");
  display.display();
}


void displayMode3() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(15, 20);
  display.printf("BOSS MOLE");
  display.setCursor(10, 30);
  display.printf("3 HP");
  display.setCursor(10, 40);
  display.printf("3 GUARD");
  display.display();
}



MoleMessage MoleInfo;
Servo myServo;
int hp;



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
  if (MoleInfo.mole == 0) {
    displayMode0();
  }

  if (MoleInfo.mole == 1) {
    displayMode1(); 
  }

  if (MoleInfo.mole == 2) {
    hp = 3; 
    displayMode2();
  }

  if (MoleInfo.mole == 3) {
    hp = 3; 
    displayMode3();
  }

  UpMole();
}


void setup() {

  // setup BUTTON
  pinMode(PUSH_BUTTON, INPUT_PULLUP);

  Serial.begin(9600); 
  WiFi.mode(WIFI_STA);


 //setup DISPLAY 
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }


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
  memcpy(peerInfoController2.peer_addr, Controller2Address, 6);
  peerInfoController2.channel = 0;
  peerInfoController2.encrypt = false;
  if (esp_now_add_peer(&peerInfoController2) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  
  //setup SERVO
  myServo.attach(SERVO_PIN);
  myServo.write(0);
  MoleInfo.mole = -1;
}





void loop() {


  if (MoleInfo.mole == -1) {
    displayNoMole();
    DownMole();
  } 
  
  if (MoleInfo.mole == 0) { // 1 HP mole
    if (Debounce() && analogRead(LDR_PIN) < 3000) { 
      // isMoleBonked = true;
      esp_err_t result = esp_now_send(Controller2Address, (uint8_t *) &MoleInfo, sizeof(MoleInfo)); 
      DownMole();
      MoleInfo.mole = -1;
    } else {
      // isBonked = false;
      //esp_err_t result = esp_now_send(Controller1Address, (uint8_t *) &MoleInfo, sizeof(MoleInfo)); 
    }
    
  } 
  
  if (MoleInfo.mole == 1) {
    if (Debounce() && analogRead(LDR_PIN) > 3000) {
      // isMoleBonked = true;
      esp_err_t result = esp_now_send(Controller2Address, (uint8_t *) &MoleInfo, sizeof(MoleInfo)); 
      DownMole();
      MoleInfo.mole = -1;
    } else {
      // isBonked = false;
    }
  }

  
  if (MoleInfo.mole == 2) { 
    if (Debounce() && analogRead(LDR_PIN) < 3000) {
      hp--;
    } 
    if (hp <= 0) { // don't come this again 
      esp_err_t result = esp_now_send(Controller2Address, (uint8_t *) &MoleInfo, sizeof(MoleInfo)); 
      DownMole();
      MoleInfo.mole = -1;
    }     
  }

  
  if (MoleInfo.mole == 3) { 
    if (Debounce() && analogRead(LDR_PIN) > 3000) {
      hp--;
    } 
    if (hp <= 0) {  // don't come this again
      esp_err_t result = esp_now_send(Controller2Address, (uint8_t *) &MoleInfo, sizeof(MoleInfo)); 
      DownMole();
      MoleInfo.mole = -1;
    }
     
  }

}
