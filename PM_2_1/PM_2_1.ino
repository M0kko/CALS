#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);
#define PCF8591_ADDR 0x48

// Структура для передачи времени
struct TimePacket {
  uint8_t header;      // 0xAA - маркер начала пакета
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
  uint16_t milliseconds;
  uint8_t adcValue;
  uint8_t checksum;    // XOR всех байтов
  uint8_t footer;      // 0x55 - маркер конца
};

TimePacket packet;
unsigned long lastTime = 0;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.begin(16, 2);
  Wire.begin();
  
  lcd.print("Binary Time Tx");
  delay(1500);
  lcd.clear();
  
  Serial.println("Binary Time Transfer Started");
  Serial.println("Packet size: " + String(sizeof(TimePacket)) + " bytes");
}

void loop() {
  if (millis() - lastTime >= 1000) {
    lastTime = millis();
    
    // Получаем текущее время из millis()
    unsigned long now = millis() / 1000;
    packet.hours = (now / 3600) % 24;
    packet.minutes = (now / 60) % 60;
    packet.seconds = now % 60;
    packet.milliseconds = millis() % 1000;
    
    // Читаем АЦП
    Wire.beginTransmission(PCF8591_ADDR);
    Wire.write(0x00);
    Wire.endTransmission();
    Wire.requestFrom(PCF8591_ADDR, 2);
    if(Wire.available() >= 2) {
      Wire.read(); // пропускаем предыдущее
      packet.adcValue = Wire.read();
    }
    
    // Формируем пакет
    packet.header = 0xAA;
    packet.footer = 0x55;
    packet.checksum = packet.header ^ packet.hours ^ packet.minutes ^ 
                      packet.seconds ^ (packet.milliseconds >> 8) ^ 
                      (packet.milliseconds & 0xFF) ^ packet.adcValue;
    
    // Отправляем бинарные данные
    Serial.write((uint8_t*)&packet, sizeof(TimePacket));
    
    // Дополнительно отправляем текстовую версию для монитора порта
    char timeStr[30];
    sprintf(timeStr, "Time: %02d:%02d:%02d.%03d ADC:%d", 
            packet.hours, packet.minutes, packet.seconds, 
            packet.milliseconds, packet.adcValue);
    
    lcd.setCursor(0, 0);
    lcd.print(timeStr);
    lcd.setCursor(0, 1);
    lcd.print("Packet sent     ");
    
    Serial.println(); // новая строка после бинарных данных
  }
}