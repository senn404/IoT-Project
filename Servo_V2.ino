#include <Servo.h>

Servo myServo;

void setup() {
  Serial.begin(9600);
  myServo.attach(6); // Servo gắn chân 6
}

void loop() {
  if (Serial.available()) {
    String color = Serial.readStringUntil('\n');
    color.trim(); // Xóa khoảng trắng thừa

    if (color.equalsIgnoreCase("RED")) {
      myServo.write(0);
    } else if (color.equalsIgnoreCase("BLUE")) {
      myServo.write(90);
    } else if (color.equalsIgnoreCase("PINK")) {
      myServo.write(180);
    }
    // Không in ra bất cứ gì
  }
}
