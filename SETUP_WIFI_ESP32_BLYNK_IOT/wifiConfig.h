#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include <EEPROM.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Ticker.h>

#define ledPin 2
#define btnPin 0
#define PUSHTIME 5000

WebServer webServer(80);
Ticker blinker;

String ssid;
String password;
int wifiMode;
unsigned long buttonPressStart = 0;
bool buttonPressed = false;
unsigned long blinkTime = millis();

const char html[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>SETTING WIFI INFORMATION</title>
  <style>
    body { display: flex; justify-content: center; align-items: center; height: 100vh; font-family: sans-serif; }
    button { width: 135px; height: 40px; margin-top: 10px; border-radius: 5px }
    label, span { font-size: 20px; }
    input, select { margin-bottom: 10px; width: 275px; height: 30px; font-size: 17px; }
  </style>
</head>
<body>
  <div>
    <h3 style="text-align: center;">SETTING WIFI INFORMATION</h3>
    <p id="info" style="text-align: center;">Scanning wifi network...!</p>
    <label>Wifi name:</label><br>
    <select id="ssid"><option>Choose wifi name!</option></select><br>
    <label>Password:</label><br>
    <input id="password" type="text"><br>
    <button onclick="saveWifi()" style="background-color: cyan;margin-right: 10px">SAVE</button>
    <button onclick="reStart()" style="background-color: pink;">RESTART</button>
  </div>
  <script>
    window.onload = () => scanWifi();
    var xhttp = new XMLHttpRequest();

    function scanWifi() {
      xhttp.onreadystatechange = () => {
        if (xhttp.readyState == 4 && xhttp.status == 200) {
          var obj = JSON.parse(xhttp.responseText);
          document.getElementById("info").innerHTML = "WiFi scan completed!";
          var select = document.getElementById("ssid");
          for (var i = 0; i < obj.length; ++i) {
            select[select.length] = new Option(obj[i], obj[i]);
          }
        }
      }
      xhttp.open("GET", "/scanWifi", true);
      xhttp.send();
    }

    function saveWifi() {
      var ssid = document.getElementById("ssid").value;
      var pass = document.getElementById("password").value;
      xhttp.onreadystatechange = () => {
        if (xhttp.readyState == 4 && xhttp.status == 200) {
          alert(xhttp.responseText);
        }
      }
      xhttp.open("GET", "/saveWifi?ssid=" + ssid + "&pass=" + pass, true);
      xhttp.send();
    }

    function reStart() {
      xhttp.onreadystatechange = () => {
        if (xhttp.readyState == 4 && xhttp.status == 200) {
          alert(xhttp.responseText);
        }
      }
      xhttp.open("GET", "/reStart", true);
      xhttp.send();
    }
  </script>
</body>
</html>
)html";

void blinkLed(uint32_t t) {
  if (millis() - blinkTime > t) {
    digitalWrite(ledPin, !digitalRead(ledPin));
    blinkTime = millis();
  }
}

void ledControl() {
  // Show different LED patterns based on button press duration
  if (buttonPressed) {
    unsigned long pressDuration = millis() - buttonPressStart;
    if (pressDuration < PUSHTIME) {
      blinkLed(200); // Fast blink during button press
    } else {
      blinkLed(50);  // Very fast blink when about to erase
    }
  } else {
    // Normal LED patterns based on WiFi mode
    if (wifiMode == 0) blinkLed(50);        // AP mode - very fast blink
    else if (wifiMode == 1) blinkLed(3000); // Connected - slow blink
    else if (wifiMode == 2) blinkLed(300);  // Connecting - medium blink
  }
}

void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  if (event == WIFI_EVENT_STA_DISCONNECTED) {
    Serial.println("Disconnected from WiFi");
    wifiMode = 2;
    WiFi.begin(ssid.c_str(), password.c_str());
  }
}

void GotIpEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  wifiMode = 1;
}

void setupWifi() {
  if (ssid != "") {
    Serial.println("-> Có ssid. Đang cố kết nối WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    WiFi.onEvent(WiFiEvent, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.onEvent(GotIpEvent, ARDUINO_EVENT_WIFI_STA_GOT_IP);
  } else {
    Serial.println("-> Không có ssid. Tạo AP WiFi cho cấu hình.");
    WiFi.mode(WIFI_AP);
    uint8_t macAddr[6];
    WiFi.softAPmacAddress(macAddr);
    String ssid_ap = "ESP32-" + String(macAddr[4], HEX) + String(macAddr[5], HEX);
    ssid_ap.toUpperCase();
    WiFi.softAP(ssid_ap.c_str());
    Serial.println("Access Point Name: " + ssid_ap);
    Serial.println("Web Server Address: " + WiFi.softAPIP().toString());
    wifiMode = 0;
  }
}

void setupWebServer() {
  webServer.on("/", [] {
    webServer.send(200, "text/html", html);
  });

  webServer.on("/scanWifi", [] {
    Serial.println("Scanning WiFi network...!");
    int wifi_nets = WiFi.scanNetworks();
    DynamicJsonDocument doc(500);
    for (int i = 0; i < wifi_nets; ++i) {
      doc.add(WiFi.SSID(i));
    }
    String wifiList;
    serializeJson(doc, wifiList);
    webServer.send(200, "application/json", wifiList);
  });

  webServer.on("/saveWifi", [] {
    String ssid_temp = webServer.arg("ssid");
    String password_temp = webServer.arg("pass");
    EEPROM.writeString(0, ssid_temp);
    EEPROM.writeString(32, password_temp);
    EEPROM.commit();
    webServer.send(200, "text/plain", "WiFi has been saved!");
  });

  webServer.on("/reStart", [] {
    webServer.send(200, "text/plain", "ESP32 is restarting!");
    delay(3000);
    ESP.restart();
  });

  webServer.begin();
}

void eraseWifiSettings() {
  Serial.println("Erasing WiFi settings from EEPROM...");
  
  // End current EEPROM session
  EEPROM.end();
  
  // Restart EEPROM
  EEPROM.begin(100);
  
  // Clear the entire EEPROM area with zeros
  for (int i = 0; i < 100; i++) {
    EEPROM.write(i, 0);
  }
  
  // Force commit the changes
  EEPROM.commit();
  
  // Wait for EEPROM to be written
  delay(500);
  
  // End and restart EEPROM again to ensure changes take effect
  EEPROM.end();
  EEPROM.begin(100);
  
  // Clear the global variables
  ssid = "";
  password = "";
  
  // Verify the erasure by reading back
  String testSSID = EEPROM.readString(0);
  String testPassword = EEPROM.readString(32);
  testSSID.trim();
  testPassword.trim();
  
  Serial.println("Verification - SSID: '" + testSSID + "' (length: " + String(testSSID.length()) + ")");
  Serial.println("Verification - Password: '" + testPassword + "' (length: " + String(testPassword.length()) + ")");
  
  // Show first 10 bytes of EEPROM for debugging
  Serial.print("EEPROM bytes 0-9: ");
  for (int i = 0; i < 10; i++) {
    Serial.print(EEPROM.read(i));
    Serial.print(" ");
  }
  Serial.println();
  
  if (testSSID.length() == 0 && testPassword.length() == 0) {
    Serial.println("WiFi settings erased successfully!");
  } else {
    Serial.println("WARNING: EEPROM may not be fully cleared!");
    // Try alternative clearing method
    Serial.println("Trying alternative clearing method...");
    for (int i = 0; i < 100; i++) {
      EEPROM.write(i, 0xFF);
    }
    EEPROM.commit();
    delay(100);
    for (int i = 0; i < 100; i++) {
      EEPROM.write(i, 0);
    }
    EEPROM.commit();
  }
}

void checkButton() {
  bool currentButtonState = (digitalRead(btnPin) == LOW);
  
  if (currentButtonState && !buttonPressed) {
    // Button just pressed
    buttonPressed = true;
    buttonPressStart = millis();
    Serial.println("Button pressed - hold for 5 seconds to erase WiFi settings");
  } else if (!currentButtonState && buttonPressed) {
    // Button just released
    buttonPressed = false;
    unsigned long pressDuration = millis() - buttonPressStart;
    
    if (pressDuration >= PUSHTIME) {
      // Button was held for 5+ seconds - erase EEPROM
      Serial.println("Button held for 5+ seconds - Erasing WiFi settings!");
      eraseWifiSettings();
      Serial.println("Restarting ESP32...");
      delay(2000);
      ESP.restart();
    } else {
      Serial.println("Button released after " + String(pressDuration) + "ms");
    }
  } else if (currentButtonState && buttonPressed) {
    // Button is being held - check if 5 seconds passed
    unsigned long pressDuration = millis() - buttonPressStart;
    if (pressDuration >= PUSHTIME) {
      // Erase immediately when 5 seconds is reached
      Serial.println("5 seconds reached - Erasing WiFi settings!");
      eraseWifiSettings();
      Serial.println("Restarting ESP32...");
      delay(2000);
      ESP.restart();
    }
  }
}

class Config {
public:
  void begin() {
    Serial.begin(115200);
    Serial.println("Starting WiFi Config...");
    
    pinMode(ledPin, OUTPUT);
    pinMode(btnPin, INPUT_PULLUP);
    blinker.attach_ms(50, ledControl);
    
    EEPROM.begin(100);
    
    // AUTOMATIC ERASE ON STARTUP - Always clear WiFi settings
    Serial.println("Auto-erasing WiFi settings on startup...");
    eraseWifiSettings();
    delay(1000); // Give time for EEPROM to be cleared
    
    // Read stored WiFi credentials
    ssid = EEPROM.readString(0);
    password = EEPROM.readString(32);

    // Debug: Show what was read from EEPROM
    Serial.println("Raw SSID from EEPROM: '" + ssid + "' (length: " + String(ssid.length()) + ")");
    Serial.println("Raw Password from EEPROM: '" + password + "' (length: " + String(password.length()) + ")");

    // Làm sạch dữ liệu nếu chuỗi không hợp lệ
    ssid.trim();
    password.trim();
    
    Serial.println("After trim - SSID: '" + ssid + "' (length: " + String(ssid.length()) + ")");
    Serial.println("After trim - Password: '" + password + "' (length: " + String(password.length()) + ")");
    
    // Additional check - if SSID contains only non-printable characters, clear it
    bool validSSID = true;
    for (int i = 0; i < ssid.length(); i++) {
      if (ssid.charAt(i) < 32 || ssid.charAt(i) > 126) {
        validSSID = false;
        break;
      }
    }
    
    if (ssid.length() == 0 || ssid == " " || ssid == "\n" || ssid == "\r" || !validSSID) {
      Serial.println("SSID is empty or invalid, clearing both credentials");
      ssid = "";
      password = "";
    }

    // Add manual erase command via Serial
    Serial.println("Type 'ERASE' in Serial Monitor to manually clear WiFi settings");

    if (ssid != "") {
      Serial.println("WiFi name: " + ssid);
      Serial.println("Password: " + password);
    } else {
      Serial.println("No WiFi credentials found");
    }

    setupWifi();
    if (wifiMode == 0) setupWebServer();
  }

  void run() {
    checkButton();
    
    // Check for manual erase command via Serial
    if (Serial.available()) {
      String command = Serial.readString();
      command.trim();
      command.toUpperCase();
      
      if (command == "ERASE") {
        Serial.println("Manual erase command received!");
        eraseWifiSettings();
        Serial.println("Restarting ESP32...");
        delay(2000);
        ESP.restart();
      } else if (command == "STATUS") {
        Serial.println("Current WiFi Status:");
        Serial.println("SSID: '" + ssid + "'");
        Serial.println("Password: '" + password + "'");
        Serial.println("WiFi Mode: " + String(wifiMode));
        Serial.println("Button State: " + String(digitalRead(btnPin)));
      }
    }
    
    if (wifiMode == 0) webServer.handleClient();
  }
} wifiConfig;

#endif