#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4);

const char* lines[3] = {
  "Temp: 23 C    ",
  "Mode: AUTO   ",
  "Status: ON   "
};

void setup() {
  lcd.init();
  lcd.init();
  lcd.backlight();
  lcd.begin(16, 2);
}

void loop() {
  // Показываем строки 0 и 1
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(lines[0]);
  lcd.setCursor(0, 1);
  lcd.print(lines[1]);
  delay(2000);

  // Показываем строки 1 и 2 (сдвигаемся вверх)
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(lines[1]);
  lcd.setCursor(0, 1);
  lcd.print(lines[2]);
  delay(2000);

  // Показываем строки 2 и 0 (для цикличности)
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(lines[2]);
  lcd.setCursor(0, 1);
  lcd.print(lines[0]);
  delay(2000);
}