#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "fix_fft.h"

#define SAMPLES 128             // Должно быть степенью 2
#define SAMPLE_WINDOW 64        // Количество бинов для отображения (половина SAMPLES)
#define MIC_PIN A0              // Аналоговый пин микрофона
#define LCD_ADDRESS 0x27        // Или 0x3F, проверьте с I2C сканером

LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);

char re[SAMPLES], im[SAMPLES];  // Массивы для реальной и мнимой частей FFT
byte magnitude[SAMPLE_WINDOW];  // Магнитуды для отображения

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Spectrum Analyzer");
  lcd.setCursor(0, 1);
  lcd.print("Ready...");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Сбор выборки аудио
  uint16_t samplingPeriod = round(1000000 * (1.0 / SAMPLES));  // ~7.8 кГц
  unsigned long startTime = micros();
  for (int i = 0; i < SAMPLES; i++) {
    while (micros() - startTime < samplingPeriod) {}  // Ждем для равномерной выборки
    startTime += samplingPeriod;
    int sample = analogRead(MIC_PIN);
    re[i] = (sample / 4) - 128;  // Масштабируем в signed char (-128..127)
    im[i] = 0;
  }

  // Выполняем FFT
  fix_fft(re, im, 7, 0);  // log2(128) = 7

  // Вычисляем магнитуды первых 64 бинов (0-~3.9 кГц)
  for (int i = 0; i < SAMPLE_WINDOW; i++) {
    magnitude[i] = sqrt(re[i] * re[i] + im[i] * im[i]);
  }

  // Очищаем LCD
  lcd.clear();

  // Отображаем бары на двух строках (по 8 бинов на строку)
  lcd.setCursor(0, 0);
  for (int i = 0; i < 8; i++) {
    int barWidth = map(magnitude[i * 8] / 2, 0, 64, 0, 16);  // Масштабируем высоту бара
    barWidth = constrain(barWidth, 0, 16);
    for (int j = 0; j < barWidth; j++) {
      lcd.print("|");
    }
    for (int j = barWidth; j < 16; j += 2) {  // Пробелы для пустоты
      lcd.print(" ");
    }
  }

  lcd.setCursor(0, 1);
  for (int i = 0; i < 8; i++) {
    int barWidth = map(magnitude[(i + 8) * 8] / 2, 0, 64, 0, 16);
    barWidth = constrain(barWidth, 0, 16);
    for (int j = 0; j < barWidth; j++) {
      lcd.print("|");
    }
    for (int j = barWidth; j < 16; j += 2) {
      lcd.print(" ");
    }
  }

  delay(50);  // Небольшая задержка для стабильности
}
