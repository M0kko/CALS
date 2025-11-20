#include <Keypad.h>
#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <LiquidCrystal_I2C.h> // Используем I2C LCD для экономии пинов
#include <RTClib.h>        // Основная библиотека для RTC

// Настройка клавиатуры 4x4
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {A1, A2, A3, A4};
byte colPins[COLS] = {9, 10, 11, 12}; // Пины, не конфликтующие с RTC

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Определения пинов для RTC (DS1302)
#define RTC_RST 8
#define RTC_DAT 7
#define RTC_CLK 6

// Глобальные объекты и переменные
LiquidCrystal_I2C lcd(0x27, 16, 2); // Адрес I2C, 16 столбцов, 2 строки
DS1302 rtc(RTC_RST, RTC_DAT, RTC_CLK);

QueueHandle_t xLuxQueue;
const TickType_t xDelay200ms = pdMS_TO_TICKS(200);
const TickType_t xDelay1000ms = pdMS_TO_TICKS(1000);
const TickType_t xDelay100ms = pdMS_TO_TICKS(100);

// Переменные для установки времени
bool settingTime = false;
int timeValues[6] = {0}; // [год, месяц, день, час, минута, секунда]
int currentField = 0;
const char* fieldNames[] = {"Year", "Month", "Day", "Hour", "Minute", "Second"};
const int fieldMax[] = {9999, 12, 31, 23, 59, 59}; // Максимальные значения
const int fieldMin[] = {2020, 1, 1, 0, 0, 0};      // Минимальные значения

// --- Структура и данные для калибровки ---
struct CalibrationPoint {
    int adc;
    float lux;
};

// Массив калибровочных данных (должен быть заполнен экспериментально)
const CalibrationPoint CalibrationData[] = {
    {200, 10.0},
    {400, 150.0},
    {600, 400.0},
    {800, 900.0},
    {1000, 1500.0}
};
const int CalibrationSize = sizeof(CalibrationData) / sizeof(CalibrationPoint);
// --- Конец калибровочных данных ---

// Прототипы функций
float calculateLuxFromADC(int D, const CalibrationPoint* data, int size);
void vTaskMeasureLuminosity(void *pvParameters);
void vTaskDisplay(void *pvParameters);
void vTaskRTC(void *pvParameters);
void vTaskKeypad(void *pvParameters);
void startTimeSetting();
void processTimeSetting(char key);
void updateTimeDisplay();
void setRTCTime();

void setup() {
  Serial.begin(9600);
  
  // Инициализация LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  
  rtc.begin();

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    lcd.clear();
    lcd.print("RTC Stopped!");
    // Установка времени компиляции
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    delay(2000);
  }
  
  // Установка времени вручную
  rtc.adjust(DateTime(2024, 5, 10, 15, 30, 0));

  // Создание очереди
  xLuxQueue = xQueueCreate(5, sizeof(float));
  
  if (xLuxQueue != NULL) {
    // Создание задач
    xTaskCreate(vTaskMeasureLuminosity, "Measure", configMINIMAL_STACK_SIZE * 2, NULL, 3, NULL);
    xTaskCreate(vTaskDisplay, "Display", configMINIMAL_STACK_SIZE * 2, NULL, 2, NULL);
    xTaskCreate(vTaskRTC, "RTC_Print", configMINIMAL_STACK_SIZE * 2, NULL, 1, NULL);
    xTaskCreate(vTaskKeypad, "Keypad", configMINIMAL_STACK_SIZE * 2, NULL, 1, NULL);
    
    // Запуск планировщика
    vTaskStartScheduler();
  } else {
    Serial.println("Ошибка создания очереди!");
    lcd.clear();
    lcd.print("Queue Error!");
  }
}

void loop() {
  
}

// --- Задача измерения освещенности ---
void vTaskMeasureLuminosity(void *pvParameters) {
  float luxValue = 0.0;
  int adcValue;
  
  for (;;) {
    adcValue = analogRead(A0);
    luxValue = calculateLuxFromADC(adcValue, CalibrationData, CalibrationSize);
    
    // Отправка в очередь. Если очередь полна - ждем вечно (portMAX_DELAY)
    if (xQueueSend(xLuxQueue, &luxValue, portMAX_DELAY) != pdPASS) {
      // В реальном проекте здесь должна быть обработка ошибки
    }
    vTaskDelay(xDelay200ms);
  }
}

// --- Задача дисплея ---
void vTaskDisplay(void *pvParameters) {
  float receivedLux;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(500); // Обновляем дисплей раз в 500 мс (2 Гц)
  
  for (;;) {
    if (!settingTime) {
      // Неблокирующее чтение из очереди
      if (xQueueReceive(xLuxQueue, &receivedLux, 0) == pdPASS) {
        // Выводим данные только если они были в очереди
        lcd.setCursor(0, 0);
        lcd.print("Lux: ");
        lcd.print(receivedLux, 1);
        lcd.print("    "); // Очистка хвоста от старых цифр
        lcd.setCursor(0, 1);
        lcd.print("RTOS Active    ");
      }
    }
    // Задержка для фиксированной частоты обновления
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

// --- Задача RTC и логирования ---
void vTaskRTC(void *pvParameters) {
  float logLux;
  
  for (;;) {
    if (!settingTime) {
      // Чтение времени из RTC
      DateTime now = rtc.now();
      
      // Попытка получить значение освещенности
      if (xQueueReceive(xLuxQueue, &logLux, 0) == pdPASS) {
        Serial.print("Time: ");
        Serial.print(now.hour()); Serial.print(":");
        if (now.minute() < 10) Serial.print("0");
        Serial.print(now.minute()); Serial.print(":");
        if (now.second() < 10) Serial.print("0");
        Serial.print(now.second()); Serial.print(" | Lux: ");
        Serial.println(logLux, 1);
      } else {
        Serial.print("Time: ");
        Serial.print(now.hour()); Serial.print(":");
        if (now.minute() < 10) Serial.print("0");
        Serial.print(now.minute()); Serial.print(":");
        if (now.second() < 10) Serial.print("0");
        Serial.println(now.second());
      }
    }
    
    vTaskDelay(xDelay1000ms);
  }
}

// Задача для обработки клавиатуры
void vTaskKeypad(void *pvParameters) {
  char key;
  
  for (;;) {
    key = keypad.getKey();
    
    if (key) {
      Serial.print("Key pressed: ");
      Serial.println(key);
      
      if (!settingTime) {
        // Обычный режим
        if (key == 'A') {
          // Начало установки времени
          startTimeSetting();
        }
      } else {
        // Режим установки времени
        processTimeSetting(key);
      }
    }
    
    vTaskDelay(xDelay100ms);
  }
}

// Начало установки времени
void startTimeSetting() {
  settingTime = true;
  currentField = 0;
  
  // Получаем текущее время для начальных значений
  DateTime now = rtc.now();
  timeValues[0] = now.year();
  timeValues[1] = now.month();
  timeValues[2] = now.day();
  timeValues[3] = now.hour();
  timeValues[4] = now.minute();
  timeValues[5] = now.second();
  
  updateTimeDisplay();
}

// Обработка нажатий клавиш в режиме установки времени
void processTimeSetting(char key) {
  if (key >= '0' && key <= '9') {
    // Ввод цифр
    int digit = key - '0';
    timeValues[currentField] = timeValues[currentField] * 10 + digit;
    
    // Проверка на превышение максимального значения
    if (timeValues[currentField] > fieldMax[currentField]) {
      timeValues[currentField] = fieldMin[currentField];
    }
    
    updateTimeDisplay();
  }
  else if (key == 'B') {
    // Переход к следующему полю
    currentField++;
    if (currentField >= 6) {
      // Все поля заполнены - устанавливаем время
      setRTCTime();
      settingTime = false;
      lcd.clear();
      lcd.print("Time Set OK!");
      vTaskDelay(pdMS_TO_TICKS(2000));
    } else {
      updateTimeDisplay();
    }
  }
  else if (key == 'C') {
    // Переход к предыдущему полю
    currentField--;
    if (currentField < 0) currentField = 0;
    updateTimeDisplay();
  }
  else if (key == 'D') {
    // Отмена установки
    settingTime = false;
    lcd.clear();
    lcd.print("Time Set Cancel");
    vTaskDelay(pdMS_TO_TICKS(1500));
  }
  else if (key == '*') {
    // Сброс текущего поля
    timeValues[currentField] = fieldMin[currentField];
    updateTimeDisplay();
  }
  else if (key == '#') {
    // Автозаполнение текущим временем
    DateTime now = rtc.now();
    int currentValues[] = {now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second()};
    timeValues[currentField] = currentValues[currentField];
    updateTimeDisplay();
  }
}

// Обновление дисплея в режиме установки времени
void updateTimeDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set ");
  lcd.print(fieldNames[currentField]);
  lcd.print(":");
  
  lcd.setCursor(0, 1);
  lcd.print(timeValues[currentField]);
  lcd.print("  B-Next C-Prev");
  
  // Мигающий курсор
  lcd.setCursor(String(timeValues[currentField]).length(), 1);
  lcd.blink();
}

// Установка времени в RTC
void setRTCTime() {
  DateTime newTime(
    timeValues[0], // год
    timeValues[1], // месяц
    timeValues[2], // день
    timeValues[3], // час
    timeValues[4], // минута
    timeValues[5]  // секунда
  );
  
  rtc.adjust(newTime);
  
  Serial.print("New time set: ");
  Serial.print(timeValues[0]); Serial.print("-");
  Serial.print(timeValues[1]); Serial.print("-");
  Serial.print(timeValues[2]); Serial.print(" ");
  Serial.print(timeValues[3]); Serial.print(":");
  Serial.print(timeValues[4]); Serial.print(":");
  Serial.println(timeValues[5]);
}

// --- Функция интерполяции (ЛИНЕЙНАЯ) ---
float calculateLuxFromADC(int D, const CalibrationPoint* data, int size) {
  // Если значение ниже минимального калибровочного
  if (D <= data[0].adc) return data[0].lux;
  // Если значение выше максимального калибровочного
  if (D >= data[size - 1].adc) return data[size - 1].lux;
  
  // Поиск интервала для интерполяции
  for (int i = 0; i < size - 1; ++i) {
    if (D >= data[i].adc && D <= data[i+1].adc) {
      float D1 = data[i].adc;
      float E1 = data[i].lux;
      float D2 = data[i+1].adc;
      float E2 = data[i+1].lux;
      
      // Линейная интерполяция
      float E = E1 + (E2 - E1) * ((float)(D - D1) / (D2 - D1));
      return E;
    }
  }
  
  return 0.0; // На случай, если что-то пошло не так
}