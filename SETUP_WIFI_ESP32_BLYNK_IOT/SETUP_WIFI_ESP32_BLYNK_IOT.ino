#define BLYNK_TEMPLATE_ID "TMPL6e8ySsrD4"
#define BLYNK_TEMPLATE_NAME "BlynkBlynk"
#define BLYNK_AUTH_TOKEN "lzr1cxxpcPtSWoG_WXjpcxvOzODDuV9w"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <BlynkSimpleEsp32.h>
#include "wifiConfig.h"

#define RXD2 16  // U2-RX
#define TXD2 17  // U2-TX
#define Buzzer 26

LiquidCrystal_I2C lcd(0x27, 16, 2);

int redCount = 0, blueCount = 0, pinkCount = 0, eCount = 0;

// IO điều khiển tương ứng với các Virtual Pins
const int startPin = 14;   // V0
const int pausePin = 27;   // V1
const int stopPin = 25;    // V2
const int historyPin = 26; // V3

bool blynkConnect = false;
unsigned long lastBlynkAttempt = 0;
unsigned long lastDataSend = 0;
const unsigned long DATA_SEND_INTERVAL = 1000; // Send data every 1 second

BLYNK_CONNECTED() {
  Serial.println("Blynk Connected!");
  Blynk.syncVirtual(V0, V1, V2, V3);
  // Send initial data when connected
  sendDataToBlynk();
}

BLYNK_WRITE(V0) {
  int p = param.asInt();
  digitalWrite(startPin, p);
  Serial.println("Start pin: " + String(p));
}

BLYNK_WRITE(V1) {
  int p = param.asInt();
  digitalWrite(pausePin, p);
  Serial.println("Pause pin: " + String(p));
}

BLYNK_WRITE(V2) {
  int p = param.asInt();
  digitalWrite(stopPin, p);
  Serial.println("Stop pin: " + String(p));
}

BLYNK_WRITE(V3) {
  int p = param.asInt();
  digitalWrite(historyPin, p);
  Serial.println("History pin: " + String(p));
}

// Function to send color counts to Blynk
void sendDataToBlynk() {
  if (blynkConnect) {
    Blynk.virtualWrite(V4, redCount);    // Red count
    Blynk.virtualWrite(V5, blueCount);   // Blue count  
    Blynk.virtualWrite(V6, pinkCount);   // Pink count
    Blynk.virtualWrite(V7, eCount);      // Error/Unknown count
    
    Serial.println("Data sent to Blynk - Red:" + String(redCount) + 
                   " Blue:" + String(blueCount) + 
                   " Pink:" + String(pinkCount) + 
                   " Error:" + String(eCount));
    
    lastDataSend = millis();
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  pinMode(Buzzer, OUTPUT);

  // Các pin điều khiển Blynk
  pinMode(startPin, OUTPUT);
  pinMode(pausePin, OUTPUT);
  pinMode(stopPin, OUTPUT);
  pinMode(historyPin, OUTPUT);

  // Initialize pins to LOW
  digitalWrite(startPin, LOW);
  digitalWrite(pausePin, LOW);
  digitalWrite(stopPin, LOW);
  digitalWrite(historyPin, LOW);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0); lcd.print("R:");
  lcd.setCursor(5, 0); lcd.print("B:");
  lcd.setCursor(10, 0); lcd.print("P:");
  lcd.setCursor(0, 1); lcd.print("E:");

  // Khởi động WiFi Manager
  wifiConfig.begin();
  
  Serial.println("System started!");
}

void loop() {
  wifiConfig.run();

  // Handle WiFi and Blynk connection
  if (WiFi.status() == WL_CONNECTED) {
    if (!blynkConnect && millis() - lastBlynkAttempt > 5000) {
      Serial.println("Connecting to Blynk cloud...");
      Blynk.config(BLYNK_AUTH_TOKEN, "blynk.cloud", 80);
      if (Blynk.connect(5000)) {
        Serial.println("Connected to Blynk cloud!");
        blynkConnect = true;
        sendDataToBlynk(); // Send initial data
      } else {
        Serial.println("Failed to connect to Blynk");
      }
      lastBlynkAttempt = millis();
    }

    if (blynkConnect) {
      Blynk.run();
      
      // Send data periodically to ensure dashboard updates
      if (millis() - lastDataSend > DATA_SEND_INTERVAL) {
        sendDataToBlynk();
      }
    }
  } else {
    // WiFi disconnected
    if (blynkConnect) {
      blynkConnect = false;
      Serial.println("WiFi disconnected, Blynk connection lost");
    }
  }

  // Handle incoming serial data
  if (Serial2.available()) {
    String msg = Serial2.readStringUntil('\n');
    msg.trim();
    msg.toUpperCase(); // Convert to uppercase for consistency
    
    Serial.println("Received: " + msg);
    
    updateColorCount(msg);
    updateLCD();
    
    // Send updated counts to Blynk immediately when new color is detected
    if (blynkConnect) {
      sendDataToBlynk();
    }
  }
}

void updateColorCount(String color) {
  if (color == "RED") {
    redCount++;
    Serial.println("Red count updated: " + String(redCount));
  }
  else if (color == "BLUE") {
    blueCount++;
    Serial.println("Blue count updated: " + String(blueCount));
  }
  else if (color == "PINK") {
    pinkCount++;
    Serial.println("Pink count updated: " + String(pinkCount));
  }
  else if (color == "UNKNOWN") {
    eCount++;
    Serial.println("Error count updated: " + String(eCount));
    
    // Non-blocking buzzer
    digitalWrite(Buzzer, HIGH);
    delay(500); // Reduced delay to prevent blocking
    digitalWrite(Buzzer, LOW);
  }
  else {
    Serial.println("Unknown color received: " + color);
  }
}

void updateLCD() {
  // Clear and update Red count
  lcd.setCursor(2, 0); 
  lcd.print("   ");
  lcd.setCursor(2, 0); 
  lcd.print(redCount);

  // Clear and update Blue count
  lcd.setCursor(7, 0); 
  lcd.print("   ");
  lcd.setCursor(7, 0); 
  lcd.print(blueCount);

  // Clear and update Pink count
  lcd.setCursor(12, 0); 
  lcd.print("   ");
  lcd.setCursor(12, 0); 
  lcd.print(pinkCount);

  // Clear and update Error count
  lcd.setCursor(2, 1); 
  lcd.print("    ");
  lcd.setCursor(2, 1); 
  lcd.print(eCount);
}