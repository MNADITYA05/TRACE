#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include "config.h"

TFT_eSPI tft = TFT_eSPI();
HardwareSerial GM805L(2);  // Serial2

#define SCANNER_RX 16
#define SCANNER_TX 17
#define LED_PIN 2

String scanBuffer = "";
String lastScanned = "";

// ✅ Forward Declarations
void sendScanTrigger();
void sendToBackend(String code);

void setup() {
  Serial.begin(115200);
  GM805L.begin(9600, SERIAL_8N1, SCANNER_RX, SCANNER_TX);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println("🔌 Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  tft.init();
  tft.setRotation(1);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n📅 WiFi Connected!");
    Serial.print("📡 IP Address: ");
    Serial.println(WiFi.localIP());

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("Scanner Ready", 120, 100);
    tft.drawString(WiFi.localIP().toString(), 120, 140);
  } else {
    Serial.println("\n❌ Failed to connect to WiFi.");
    tft.fillScreen(TFT_RED);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.drawString("WiFi Failed", 120, 120);
    return;
  }

  delay(1000);
  sendScanTrigger();
}

void loop() {
  while (GM805L.available()) {
    char c = GM805L.read();
    if (isPrintable(c)) scanBuffer += c;
  }

  if (scanBuffer.length() >= 15) {
    Serial.print("📩 Raw buffer: ");
    Serial.println(scanBuffer);

    String eanCode = "";
    for (int i = scanBuffer.length() - 13; i >= 0; i--) {
      bool allDigits = true;
      for (int j = 0; j < 13; j++) {
        if (!isDigit(scanBuffer[i + j])) {
          allDigits = false;
          break;
        }
      }
      if (allDigits) {
        eanCode = scanBuffer.substring(i, i + 13);
        break;
      }
    }

    if (eanCode.length() == 13 && eanCode != lastScanned) {
      Serial.print("📦 EAN-13: ");
      Serial.println(eanCode);
      digitalWrite(LED_PIN, HIGH); delay(50); digitalWrite(LED_PIN, LOW);
      sendToBackend(eanCode);
      lastScanned = eanCode;
    }

    scanBuffer = "";
    delay(300);
    sendScanTrigger();
  }

  if (scanBuffer.length() > 30) scanBuffer = "";
}

void sendScanTrigger() {
  byte cmd[] = {0x7E, 0x00, 0x08, 0x01, 0x00, 0x02, 0x01, 0xAB, 0xCD};
  GM805L.write(cmd, sizeof(cmd));
  Serial.println("📤 Trigger sent to GM805L...");
}

void sendToBackend(String code) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(BACKEND_URL);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(10000);  // 10 seconds

    String json = "{\"barcode\": \"" + code + "\"}";
    int httpCode = http.POST(json);

    if (httpCode == 200) {
      String response = http.getString();
      Serial.println("✅ Response: " + response);

      DynamicJsonDocument doc(768);
      if (deserializeJson(doc, response) == DeserializationError::Ok) {
        String status = doc["status"].as<String>();

        if (status == "found") {
          String product = doc["product_id"] | "N/A";
          String mfg = doc["manufacturing_date"] | "N/A";
          String barcode = doc["barcode"] | "N/A";
          String quality = doc["ml_result"]["quality_status"] | "unknown";

          Serial.println("📦 Product: " + product);
          Serial.println("🏷️ MFG: " + mfg);
          Serial.println("📊 Quality: " + quality);

          tft.fillScreen(TFT_BLACK);
          tft.setTextSize(2);
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
          tft.drawString("✅ MATCHED", 120, 30);

          tft.setTextColor(TFT_WHITE, TFT_BLACK);
          tft.setTextSize(1);
          tft.drawString("Product: " + product, 120, 60);
          tft.drawString("MFG: " + mfg, 120, 80);
          tft.drawString("Barcode: " + barcode, 120, 100);
          tft.drawString("Quality: " + quality, 120, 120);
        } else {
          Serial.println("🚫 Barcode Not Found [status in JSON]");
          tft.fillScreen(TFT_RED);
          tft.setTextColor(TFT_WHITE, TFT_RED);
          tft.drawString("❌ NOT FOUND", 120, 100);
        }
      } else {
        Serial.println("❌ JSON Parse Error");
        tft.fillScreen(TFT_RED);
        tft.drawString("JSON Error", 120, 120);
      }
    } else if (httpCode == 404) {
      Serial.println("🚫 Barcode Not Found [404]");
      tft.fillScreen(TFT_RED);
      tft.setTextColor(TFT_WHITE, TFT_RED);
      tft.drawString("❌ NOT FOUND", 120, 100);
    } else {
      Serial.print("❌ Backend Error: ");
      Serial.println(httpCode);
      tft.fillScreen(TFT_RED);
      tft.setTextColor(TFT_WHITE, TFT_RED);
      tft.setTextSize(2);
      tft.drawString("Server Error", 120, 100);
      tft.drawString("Code: " + String(httpCode), 120, 130);
    }

    http.end();
  }
}
