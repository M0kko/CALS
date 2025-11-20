#include <Arduino_FreeRTOS.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <RTClib.h>  // Подключаем RTClib
#include <Keypad.h>
#include <EEPROM.h>
#include <queue.h>
#include <semphr.h>

DS1302 rtc;

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// КЛАВИАТУРА (Настройка для 4x3 или 4x4)
const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};
byte rowPins[ROWS] = {9, 10, 11, 12}; 
byte colPins[COLS] = {A1, A2, A3, A4}; 
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// --- ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ RTOS ---
QueueHandle_t xLuxQueue;      
QueueHandle_t xLogQueue;      
SemaphoreHandle_t xRTCMutex;  // Обязательно нужен мьютекс для RTC

// Структура для логгера (Задание 9)
struct LogData {
  uint8_t day;
  uint8_t month;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  float lux;
  char label[8]; // "ManRec" + \0
};

// Калибровочные данные
struct CalibrationPoint {
  int adc;
  float lux;
};
const CalibrationPoint CalibrationData[] = {
  {0, 0.0}, {50, 1.0}, {200, 50.0}, {450, 200.0}, {700, 800.0}, {1023, 2500.0}
};
const int CalibrationSize = sizeof(CalibrationData) / sizeof(CalibrationPoint);

// --- ПРОТОТИПЫ ---
float calculateLuxFromADC(int adcValue);
void vTaskMeasure(void *pvParameters);
void vTaskDisplayUI(void *pvParameters);
void vTaskRTCReport(void *pvParameters);
void vTaskEEPROM(void *pvParameters);

// --- SETUP ---
void setup() {
  Serial.begin(9600);
  
  // Инициализация LCD
  lcd.init();
  lcd.backlight();
  
  delay(500); // Даем модулю проснуться
  // Инициализация RTC
  rtc.begin();
  
  // Проверка, запущены ли часы. Если нет - ставим время компиляции.
  if (!rtc.isrunning()) {
    Serial.println(F("RTC is NOT running!"));
    // Следующая строка устанавливает время компиляции скетча
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Создание ресурсов ОС
  xLuxQueue = xQueueCreate(1, sizeof(float)); 
  xLogQueue = xQueueCreate(2, sizeof(LogData)); 
  xRTCMutex = xSemaphoreCreateMutex();

  if (xLuxQueue && xLogQueue && xRTCMutex) {
    // Создание задач
    // Распределение стека критично для Uno (всего 2Кб RAM)
    xTaskCreate(vTaskMeasure, "Meas", 100, NULL, 3, NULL);
    xTaskCreate(vTaskDisplayUI, "UI", 210, NULL, 2, NULL); // UI требует больше стека
    xTaskCreate(vTaskRTCReport, "SerRTC", 130, NULL, 1, NULL);
    xTaskCreate(vTaskEEPROM, "Log", 100, NULL, 1, NULL);
    
    Serial.println(F("System Started (RTClib version)"));
  } else {
    lcd.print("RAM Fail");
    while(1);
  }
}

void loop() {}

// --- 1. ЗАДАЧА ИЗМЕРЕНИЯ ---
void vTaskMeasure(void *pvParameters) {
  float lux;
  for (;;) {
    int adc = analogRead(A0);
    lux = calculateLuxFromADC(adc);
    xQueueOverwrite(xLuxQueue, &lux);
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

// --- 2. ЗАДАЧА ИНТЕРФЕЙСА (LCD + KEYPAD) ---
void vTaskDisplayUI(void *pvParameters) {
  float currentLux = 0.0;
  char timeBuffer[10]; // "HH:MM:SS"
  
  for (;;) {
    // Получаем Lux
    xQueueReceive(xLuxQueue, &currentLux, 0);

    // Получаем Время (RTClib создает объект DateTime, это требует защиты мьютексом)
    DateTime now;
    if (xSemaphoreTake(xRTCMutex, pdMS_TO_TICKS(100)) == pdPASS) {
      now = rtc.now();
      xSemaphoreGive(xRTCMutex);
    }
    
    // Форматирование времени вручную (sprintf ест много памяти, но удобен)
    sprintf(timeBuffer, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());

    // Вывод на LCD
    lcd.setCursor(0, 0);
    lcd.print("L:"); lcd.print((int)currentLux); lcd.print("   ");
    lcd.setCursor(8, 0);
    lcd.print(timeBuffer);

    lcd.setCursor(0, 1);
    lcd.print("[*]Set [#]Log");

    // Обработка клавиатуры
    char key = keypad.getKey();
    
    // --- УСТАНОВКА ВРЕМЕНИ (Задание 8) ---
    if (key == '*') {
      lcd.clear();
      lcd.print("Set Time HH:MM");
      lcd.setCursor(0, 1);
      
      char inputBuf[5]; // 4 цифры + 0
      byte count = 0;
      
      // Простой цикл ввода (блокирующий эту задачу, но не всю ОС)
      while (count < 4) {
        char k = keypad.getKey();
        if (k >= '0' && k <= '9') {
          inputBuf[count] = k;
          lcd.print(k);
          count++;
        }
        // Важно: даем подышать другим задачам (RTC в serial, измерениям)
        vTaskDelay(pdMS_TO_TICKS(50)); 
      }
      
      int h = (inputBuf[0]-'0')*10 + (inputBuf[1]-'0');
      int m = (inputBuf[2]-'0')*10 + (inputBuf[3]-'0');
      
      if (xSemaphoreTake(xRTCMutex, portMAX_DELAY) == pdPASS) {
        // RTClib требует полной даты для установки времени.
        // Мы берем текущую дату и подставляем новое время.
        DateTime current = rtc.now();
        rtc.adjust(DateTime(current.year(), current.month(), current.day(), h, m, 0));
        xSemaphoreGive(xRTCMutex);
      }
      lcd.clear();
    }
    
    // --- ЗАПИСЬ В LOG (Задание 9) ---
    else if (key == '#') {
      LogData entry;
      entry.lux = currentLux;
      entry.day = now.day();
      entry.month = now.month();
      entry.hour = now.hour();
      entry.minute = now.minute();
      entry.second = now.second();
      strcpy(entry.label, "ManRec");
      
      xQueueSend(xLogQueue, &entry, 0);
      
      lcd.setCursor(0, 1);
      lcd.print("Saved to EEPROM");
      vTaskDelay(pdMS_TO_TICKS(600));
    }

    vTaskDelay(pdMS_TO_TICKS(150));
  }
}

// --- 3. ЗАДАЧА RTC REPORT (SERIAL) ---
void vTaskRTCReport(void *pvParameters) {
  for (;;) {
    if (xSemaphoreTake(xRTCMutex, pdMS_TO_TICKS(200)) == pdPASS) {
      DateTime now = rtc.now();
      xSemaphoreGive(xRTCMutex); // Сразу отдаем мьютекс, чтобы не держать UI
      
      Serial.print(F("RTC: "));
      Serial.print(now.hour()); Serial.print(':');
      Serial.print(now.minute()); Serial.print(':');
      Serial.println(now.second());
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// --- 4. ЗАДАЧА EEPROM ---
void vTaskEEPROM(void *pvParameters) {
  LogData data;
  int addr = 0;
  
  for (;;) {
    if (xQueueReceive(xLogQueue, &data, portMAX_DELAY) == pdPASS) {
      Serial.print(F("Writing Log... "));
      
      EEPROM.put(addr, data);
      
      Serial.print(F("Addr: ")); Serial.println(addr);
      
      addr += sizeof(LogData);
      if (addr > (int)(EEPROM.length() - sizeof(LogData))) addr = 0;
    }
  }
}

// --- МАТЕМАТИКА ---
float calculateLuxFromADC(int adcValue) {
   if (adcValue <= CalibrationData[0].adc) return CalibrationData[0].lux;
   if (adcValue >= CalibrationData[CalibrationSize - 1].adc) return CalibrationData[CalibrationSize - 1].lux;
  
   for (int i = 0; i < CalibrationSize - 1; ++i) {
     if (adcValue >= CalibrationData[i].adc && adcValue <= CalibrationData[i + 1].adc) {
       float x1 = CalibrationData[i].adc; float y1 = CalibrationData[i].lux;
       float x2 = CalibrationData[i+1].adc; float y2 = CalibrationData[i+1].lux;
       return y1 + (adcValue - x1) * (y2 - y1) / (x2 - x1);
     }
   }
   return 0.0;
}