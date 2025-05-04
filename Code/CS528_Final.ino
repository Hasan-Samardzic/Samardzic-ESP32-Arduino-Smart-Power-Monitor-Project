// ================================
// Hasan Samardzic                 
// Arduino Program CS528 Final     
// Wireless Smart Power Usage Meter 
// ================================

#include <Wire.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_INA219.h>
#include <WebServer.h>

// ============================
//  WiFi Credentials
// ============================
const char* ssid = "XXXX";                       //Your WiFi name
const char* password = "XXXX";                  // Your WiFi password

// ============================
//  TCP Server Settings
// ============================
const char* serverIP = "XXXX";             //Your PC IP address
const uint16_t serverPort = 12345;        // TCP Port (match Python listener)

// ============================
//  Hardware Objects
// ============================
Adafruit_INA219 ina219;
LiquidCrystal_I2C lcd(0x27, 16, 2);
WebServer webServer(80);
WiFiClient client;

// ============================
//  RGB LED Pins
// ============================
const int redPin = 2;
const int greenPin = 3;
const int bluePin = 4;

// ============================
//  Measurement Variables
// ============================
float busVoltage = 0.0;
float current_mA = 0.0;
float power_mW = 0.0;
unsigned long lastUpdateTime = 0;
unsigned long lastSendTime = 0;

// ======================================================
//  Setup
// ======================================================
void setup() {
  Serial.begin(115200);
  Wire.begin();
  ina219.begin();
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Starting...");

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  connectWiFi();
  startWebServer();
}

// ======================================================
//  Main Loop
// ======================================================
void loop() {
  webServer.handleClient();

  if (millis() - lastUpdateTime >= 1000) {
    updateReadings();
    lastUpdateTime = millis();
  }

  if (millis() - lastSendTime >= 1000) {
    sendToServer();
    lastSendTime = millis();
  }
}

// ======================================================
//  WiFi Connection Logic
// ======================================================
void connectWiFi() {
  WiFi.begin(ssid, password);
  lcd.clear();
  lcd.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(redPin, HIGH);
    delay(500);
    digitalWrite(redPin, LOW);
    delay(500);
    Serial.println("Connecting to WiFi...");
  }

  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, HIGH);
  lcd.clear();
  lcd.print("WiFi Connected");
  delay(1000);
  lcd.clear();
}

// ======================================================
//  Update INA219 Sensor Readings + LCD Display
// ======================================================
void updateReadings() {
  busVoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();

  lcd.setCursor(0, 0);
  lcd.print("V:");
  lcd.print(busVoltage, 2);
  lcd.print(" I:");
  lcd.print(current_mA, 1);

  lcd.setCursor(0, 1);
  lcd.print("P:");
  lcd.print(power_mW, 1);
  lcd.print("mW ");
}

// ======================================================
//  TCP Socket Send to Python Server
// ======================================================
void sendToServer() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      Serial.println("Connecting to Server...");
      client.connect(serverIP, serverPort);
    }
    
    if (client.connected()) {
      String payload = "Voltage=" + String(busVoltage, 2) + "V, Current=" + String(current_mA, 1) + "mA, Power=" + String(power_mW, 1) + "mW";
      client.println(payload);
      Serial.println("Sent: " + payload);

      digitalWrite(bluePin, HIGH);
      delay(100);
      digitalWrite(bluePin, LOW);
    } else {
      Serial.println("Server not connected.");
    }
  }
}

// ======================================================
//  Web Server (Serve HTML Webpage)
// ======================================================
void startWebServer() {
  webServer.on("/", handleRoot);
  webServer.begin();
  Serial.println("HTTP server started");
}

// ======================================================
//  HTML Page 
// ======================================================
void handleRoot() {
  String page = R"rawliteral(
  <!DOCTYPE html>
  <html lang="en">
  <head>
      <meta charset="UTF-8">
      <meta http-equiv="refresh" content="2">
      <title>⚡Samardzic⚡</title>
      <style>
          body {
              background-color: #0d1117;
              font-family: 'Source Code Pro', monospace;
              color: #c9d1d9;
              margin: 0;
              padding: 0;
              display: flex;
              flex-direction: column;
              min-height: 100vh;
              align-items: center;
          }
          h1 {
              text-align: center;
              margin: 30px 0 20px 0;
              font-size: 2.8rem;
              color: #58a6ff;
          }
          .container {
              flex: 1;
              display: flex;
              justify-content: center;
              align-items: center;
              width: 100%;
          }
          .card {
              background-color: #161b22;
              padding: 50px 60px;
              border-radius: 15px;
              box-shadow: 0 8px 20px rgba(0,0,0,0.5);
              text-align: center;
              font-size: 26px;
              min-width: 400px;
          }
          .status {
              margin-bottom: 25px;
          }
          .status span {
              font-weight: bold;
          }
          .green {
              color: #3fb950;
          }
          .red {
              color: #f85149;
          }
          .timestamp {
              margin-top: 25px;
              font-size: 16px;
              color: #8b949e;
          }
      </style>
  </head>
  <body>
      <h1>⚡Samardzic CS528 Final Project⚡</h1>
      <div class="container">
        <div class="card">
          <div class="status">
              WiFi Status: <span class=")rawliteral";

  if (WiFi.status() == WL_CONNECTED) {
    page += "green";
  } else {
    page += "red";
  }

  page += R"rawliteral(">)rawliteral";

  if (WiFi.status() == WL_CONNECTED) {
    page += "Connected";
  } else {
    page += "Disconnected";
  }

  page += R"rawliteral(</span><br>
              Server Status: <span class=")rawliteral";

  if (client.connected()) {
    page += "green";
  } else {
    page += "red";
  }

  page += R"rawliteral(">)rawliteral";

  if (client.connected()) {
    page += "Connected";
  } else {
    page += "Disconnected";
  }

  page += R"rawliteral(</span>
          </div>
          <div class="data">
              <strong>Voltage:</strong> )rawliteral" + String(busVoltage, 2) + R"rawliteral( V<br>
              <strong>Current:</strong> )rawliteral" + String(current_mA, 1) + R"rawliteral( mA<br>
              <strong>Power:</strong> )rawliteral" + String(power_mW, 1) + R"rawliteral( mW
          </div>
          <div class="timestamp">
              Uptime: )rawliteral" + String(millis() / 1000) + R"rawliteral( seconds
          </div>
        </div>
      </div>
  </body>
  </html>
  )rawliteral";

  webServer.send(200, "text/html", page);
}