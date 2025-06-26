#define RXD2 16  // U2-RX
#define TXD2 17  // U2-TX

void setup() {
  Serial.begin(115200); // Để debug
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); // UART2
}

void loop() {
  if (Serial2.available()) {
    String msg = Serial2.readStringUntil('\n');
    
  }
}
