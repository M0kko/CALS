#include <LiquidCrystal_I2C.h>  // Библиотека для LCD дисплея по I2C

// Настройка дисплея (адрес I2C, кол-во столбцов, строк)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Пины
const int PHOTO_PIN = A0;      // Пин фоторезистора
const int LED_PIN = 13;        // Встроенный светодиод для индикации

// Переменные
int adcValue = 0;              // Сырое значение АЦП
int lightLevel = 0;            // Уровень освещенности в %
unsigned long previousMillis = 0;
const long INTERVAL = 1000;    // Интервал обновления (мс)

void setup() {
  // Инициализация Serial
  Serial.begin(9600);
  
  // Инициализация дисплея
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  // Настройка пинов
  pinMode(LED_PIN, OUTPUT);
  
  // Приветственное сообщение
  Serial.println("Система мониторинга освещенности");
  Serial.println("=================================");
  
  lcd.setCursor(0, 0);
  lcd.print("Light Monitor");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  
  delay(2000);
  lcd.clear();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Обновляем данные каждые INTERVAL миллисекунд
  if (currentMillis - previousMillis >= INTERVAL) {
    previousMillis = currentMillis;
    
    // Чтение АЦП
    readLightSensor();
    
    // Вывод в Serial
    printToSerial();
    
    // Вывод на дисплей
    printToLCD();
    
    // Мигаем светодиодом для индикации работы
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
}

void readLightSensor() {
  // Читаем значение АЦП (0-1023)
  adcValue = analogRead(PHOTO_PIN);
  
  // Преобразуем в проценты (инвертируем, так чем больше света - тем меньше сопротивление)
  lightLevel = map(adcValue, 0, 1023, 100, 0);
  lightLevel = constrain(lightLevel, 0, 100);  // Ограничиваем диапазон 0-100%
}

void printToSerial() {
  Serial.print("ADC: ");
  Serial.print(adcValue);
  Serial.print(" | Освещенность: ");
  Serial.print(lightLevel);
  Serial.print("% | ");
  
  // Графическая индикация
  int bars = map(lightLevel, 0, 100, 0, 20);
  Serial.print("[");
  for (int i = 0; i < 20; i++) {
    if (i < bars) {
      Serial.print("=");
    } else {
      Serial.print(" ");
    }
  }
  Serial.println("]");
}

void printToLCD() {
  lcd.setCursor(0, 0);
  lcd.print("ADC:");
  lcd.print(adcValue);
  lcd.print("    ");  // Очищаем оставшиеся символы
  
  lcd.setCursor(0, 1);
  lcd.print("Light:");
  lcd.print(lightLevel);
  lcd.print("% ");
  
  // Простая графическая индикация на дисплее
  int bars = map(lightLevel, 0, 100, 0, 8);
  lcd.print("[");
  for (int i = 0; i < 8; i++) {
    if (i < bars) {
      lcd.print("=");
    } else {
      lcd.print(" ");
    }
  }
  lcd.print("]");
}