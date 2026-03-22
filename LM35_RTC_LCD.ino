#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <ESP8266WiFi.h>
#include "ThingSpeak.h"

// ─── CONFIG ───────────────────────────────────────────
const char* WIFI_SSID     = "iman";
const char* WIFI_PASS     = "iman@12345";

unsigned long TS_CHANNEL  =;       // Your ThingSpeak Channel ID
const char*   TS_API_KEY  = "; // Write API Key
// ──────────────────────────────────────────────────────

LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;
WiFiClient client;

const int LM35_PIN = A0;
unsigned long lastUpload = 0;
const unsigned long UPLOAD_INTERVAL = 15000; // ThingSpeak min = 15 sec

void connectWiFi() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    lcd.setCursor(attempts % 16, 1);
    lcd.print(".");
    attempts++;
  }
  lcd.clear();
  if (WiFi.status() == WL_CONNECTED) {
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected!");
    delay(1000);
  } else {
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed!");
    delay(2000);
  }
}

void setup() {
  lcd.init();
  lcd.backlight();

  if (!rtc.begin()) {
    lcd.setCursor(0, 0);
    lcd.print("RTC not found!");
    while (1);
  }

  // Uncomment ONCE to set time, then comment out again and re-upload
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  connectWiFi();
  ThingSpeak.begin(client);
}

void loop() {
  DateTime now = rtc.now();

  // --- Read LM35 Temperature ---
  int raw = analogRead(LM35_PIN);
  float voltage = raw * (3.3 / 1023.0); // ESP8266 runs on 3.3V
  float tempC = voltage * 100.0;

  // --- Row 0: Date + Temp ---
  lcd.setCursor(0, 0);
  if (now.day()   < 10) lcd.print("0");
  lcd.print(now.day());
  lcd.print("/");
  if (now.month() < 10) lcd.print("0");
  lcd.print(now.month());
  lcd.print("/");
  lcd.print(now.year());
  lcd.setCursor(11, 0);
  lcd.print((int)tempC);
  lcd.print((char)223);
  lcd.print("C ");

  // --- Row 1: Time + WiFi status ---
  lcd.setCursor(0, 1);
  if (now.hour()   < 10) lcd.print("0");
  lcd.print(now.hour());
  lcd.print(":");
  if (now.minute() < 10) lcd.print("0");
  lcd.print(now.minute());
  lcd.print(":");
  if (now.second() < 10) lcd.print("0");
  lcd.print(now.second());
  lcd.setCursor(9, 1);
  lcd.print(WiFi.status() == WL_CONNECTED ? " [Online]" : "[Offline]");

  // --- Upload to ThingSpeak every 15 sec ---
  if (millis() - lastUpload >= UPLOAD_INTERVAL) {
    if (WiFi.status() != WL_CONNECTED) connectWiFi();

    ThingSpeak.setField(1, tempC);               // Field 1: Temperature
    ThingSpeak.setField(2, (int)now.hour());     // Field 2: Hour
    ThingSpeak.setField(3, (int)now.minute());   // Field 3: Minute

    int result = ThingSpeak.writeFields(TS_CHANNEL, TS_API_KEY);

    lcd.setCursor(0, 1);
    if (result == 200) {
      lcd.print("  Uploaded OK!  ");
    } else {
      lcd.print("  Upload Fail   ");
    }
    delay(1000);
    lastUpload = millis();
  }

  delay(1000);
}
}
