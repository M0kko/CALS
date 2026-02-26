#include <DHT11.h>      // подключаем библиотеку для датчика
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);
DHT11 dht11(2);  // сообщаем на каком порту будет датчик

void setup() {
  Serial.begin(9600);   // подключаем монитор порта
  lcd.init();
  lcd.backlight();
  lcd.begin(16, 2);
  Wire.begin();
}

void loop() {
   // считываем температуру (t) и влажность (h)
   float h = dht11.readHumidity();
   float t = dht11.readTemperature();
   
   lcd.setCursor(0,0);
   lcd.print("Humidity: ");
   lcd.print(h);

   lcd.setCursor(0,1);
   lcd.print("Temperature: ");
   lcd.print(t);

   delay(1000);
   lcd.clear();
}