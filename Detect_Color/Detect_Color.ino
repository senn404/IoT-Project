#include <Wire.h>
#include "Adafruit_TCS34725.h"

// Khởi tạo cảm biến với thời gian tích hợp và độ khuếch đại
Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_50MS,
  TCS34725_GAIN_4X
);

void setup() {
  Serial.begin(9600);
  if (tcs.begin()) {
    Serial.println("TCS34725 found.");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
}

// Hàm phân loại màu theo RGB
String classifyColorRGB(float r, float g, float b) {
  // RED: R rất cao, G & B thấp rõ rệt
  if (r > 170 && r > g + 50 && r > b + 50) {
    return "RED";
  }
  // PINK: R cao, G thấp, B trung bình, không quá chênh lệch
  else if (r > 130 ) {
    return "PINK";
  }
  // BLUE: B cao, G thấp, và vượt R rõ
  else if (b > 90 && g < 100 && b > r + 20) {
    return "BLUE";
  } else {
    return "UNKNOWN";
  }
}

void loop() {
  uint16_t rRaw, gRaw, bRaw, cRaw;
  tcs.getRawData(&rRaw, &gRaw, &bRaw, &cRaw);

  float sum = cRaw;
  float r = rRaw / sum * 255.0;
  float g = gRaw / sum * 255.0;
  float b = bRaw / sum * 255.0;

  String detected = classifyColorRGB(r, g, b);
  Serial.println(detected);

  delay(3000);
}