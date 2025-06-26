#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define RXD2 16  // U2-RX
#define TXD2 17  // U2-TX

LiquidCrystal_I2C lcd(0x27, 16, 2);

int redCount = 0, blueCount = 0, pinkCount = 0;

void setup() {
  Serial.begin(115200); // debug
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); // UART2

  lcd.init();
  lcd.backlight();

  // In form cố định lên LCD
  lcd.setCursor(0, 0); lcd.print("R:");
  lcd.setCursor(5, 0); lcd.print("B:");
  lcd.setCursor(10, 0); lcd.print("P:");
  lcd.setCursor(0, 1); lcd.print("Total:");
}

void loop() {
  if (Serial2.available()) {
    String msg = Serial2.readStringUntil('\n');
    msg.trim();  // loại bỏ khoảng trắng, xuống dòng

    Serial.println("Received: " + msg); // in ra để kiểm tra

    // Xử lý chuỗi nhận được
    updateColorCount(msg);
    updateLCD();
  }
}

// Giả định: Arduino gửi sang ESP32 chuỗi là tên màu, ví dụ "RED", "BLUE", "PINK"
void updateColorCount(String color) {
  if (color == "RED") redCount++;
  else if (color == "BLUE") blueCount++;
  else if (color == "PINK") pinkCount++;
  // nếu là UNKNOWN thì bỏ qua
}

void updateLCD() {
  int total = redCount + blueCount + pinkCount;

  // Xóa và cập nhật dòng 1
  lcd.setCursor(2, 0); lcd.print("   "); // clear R
  lcd.setCursor(2, 0); lcd.print(redCount);

  lcd.setCursor(7, 0); lcd.print("   "); // clear B
  lcd.setCursor(7, 0); lcd.print(blueCount);

  lcd.setCursor(12, 0); lcd.print("   "); // clear P
  lcd.setCursor(12, 0); lcd.print(pinkCount);

  // Xóa và cập nhật dòng 2
  lcd.setCursor(7, 1); lcd.print("     "); // clear Total
  lcd.setCursor(7, 1); lcd.print(total);
}