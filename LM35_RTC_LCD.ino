





#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

// LCD: I2C address 0x27, 16 columns, 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;

const int LM35_PIN = A0;

void setup() {
  lcd.init();
  lcd.backlight();

  if (!rtc.begin()) {
    lcd.setCursor(0, 0);
    lcd.print("RTC not found!");
    while (1);
  }

  // Uncomment the line below ONCE to set RTC time, then comment it out again
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void loop() {
  DateTime now = rtc.now();

  // --- Read LM35 Temperature ---
  int raw = analogRead(LM35_PIN);
  float voltage = raw * (5.0 / 1023.0);
  float tempC = voltage * 100.0;

  // --- Row 0: Date ---
  lcd.setCursor(0, 0);
  // Format: DD/MM/YYYY
  if (now.day()   < 10) lcd.print("0");
  lcd.print(now.day());
  lcd.print("/");
  if (now.month() < 10) lcd.print("0");
  lcd.print(now.month());
  lcd.print("/");
  lcd.print(now.year());

  // Temp on same row (right side): e.g. "25C"
  lcd.setCursor(11, 0);
  lcd.print((int)tempC);
  lcd.print((char)223); // degree symbol
  lcd.print("C ");

  // --- Row 1: Time ---
  lcd.setCursor(0, 1);
  // Format: HH:MM:SS
  if (now.hour()   < 10) lcd.print("0");
  lcd.print(now.hour());
  lcd.print(":");
  if (now.minute() < 10) lcd.print("0");
  lcd.print(now.minute());
  lcd.print(":");
  if (now.second() < 10) lcd.print("0");
  lcd.print(now.second());

  delay(1000);
}