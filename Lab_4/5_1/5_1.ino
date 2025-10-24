#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4);

void setup(){
  lcd.init();                      // initialize the lcd 
  lcd.init();
  lcd.backlight();
  lcd.begin(16,2); 
  lcd.print("LCD 16x2 Demo"); 
  lcd.setCursor(0,1); 
  lcd.print("Line 2");
}

void loop(){ }