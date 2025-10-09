#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- Настройки LCD ---
LiquidCrystal_I2C lcd(0x27, 20, 4);

// --- Настройки Клавиатуры ---
const int rowPins[4] = {7, 6, 5, 4}; // Строки (подключены к диодам)
const int colPins[4] = {11, 10, 9, 8}; // Столбцы
const int KEY_INT_PIN = 2; // D2 - Внешнее прерывание 0

char keys[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

volatile unsigned long timerCount = 0;
volatile bool scanFlag = false; // Флаг, установленный INT0
volatile bool keyProcessed = true;
volatile char lastKey = '\0';

// --- Обработчик прерывания Таймера1 (Обновление LCD каждые 500 мс) ---
ISR(TIMER1_COMPA_vect) {
    timerCount++;
}

// --- Обработчик внешнего прерывания (Запуск сканирования) ---
void startScan() {
    static unsigned long lastInterruptTime = 0;
    unsigned long interruptTime = millis();
    
    // Подавление дребезга
    if (interruptTime - lastInterruptTime > 50) {
        scanFlag = true;
        keyProcessed = false;
    }
    lastInterruptTime = interruptTime;
}

void setupKeyboardPins() {
    for(int i = 0; i < 4; i++) {
        pinMode(rowPins[i], OUTPUT);
        digitalWrite(rowPins[i], HIGH); // Строки по умолчанию HIGH
    }
    for(int i = 0; i < 4; i++) {
        pinMode(colPins[i], INPUT_PULLUP); // Используем внутреннюю подтяжку
    }
}

void setupTimer1() {
    // Настройка Timer1 для прерывания каждые 500 мс
    noInterrupts();
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    
    OCR1A = 7812; // = (16*10^6) / (1024 * 2) - 1 (для 2 Гц)
    TCCR1B |= (1 << WGM12); // CTC mode
    TCCR1B |= (1 << CS12) | (1 << CS10); // Prescaler 1024
    TIMSK1 |= (1 << OCIE1A); // Включение прерывания
    interrupts();
}

// Функция полного сканирования клавиатуры
char scanKeyboard() {
    for (int i = 0; i < 4; i++) {
        // Активируем строку i (подаем LOW)
        digitalWrite(rowPins[i], LOW);

        // Небольшая задержка для стабилизации (критично при использовании схемы ИЛИ)
        delayMicroseconds(100);

        for (int j = 0; j < 4; j++) {
            if (digitalRead(colPins[j]) == LOW) {
                // Клавиша найдена: keys[i][j]
                char key = keys[i][j];
                
                // Ждем отпускания клавиши (антидребезг)
                delay(50);
                while(digitalRead(colPins[j]) == LOW) {
                    delay(1);
                }
                
                // Отключаем строку перед возвратом
                digitalWrite(rowPins[i], HIGH);
                return key;
            }
        }
        // Деактивируем строку перед переходом к следующей
        digitalWrite(rowPins[i], HIGH);
    }
    return 0; // Ничего не найдено
}

void setup() {
    Serial.begin(9600);
    lcd.init();
    lcd.backlight();
    setupKeyboardPins();
    setupTimer1();

    // --- Настройка Внешнего прерывания (D2) ---
    pinMode(KEY_INT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(KEY_INT_PIN), startScan, FALLING);

    interrupts(); // Включаем глобальные прерывания

    lcd.setCursor(0, 0);
    lcd.print("System Ready");
    lcd.setCursor(0, 1);
    lcd.print("Count: 0");
    
    Serial.println("Задание 4: Комбинация прерываний");
    Serial.println("Система готова");
}

void loop() {
    static unsigned long lastDisplayUpdate = 0;
    
    // 1. Обновление счетчика на LCD (из таймера)
    if (timerCount != lastDisplayUpdate) {
        lcd.setCursor(0, 0);
        lcd.print("              "); // Очистка области
        lcd.setCursor(7, 1);
        lcd.print(timerCount);
        lastDisplayUpdate = timerCount;
        
        // Также выводим время работы
        lcd.setCursor(0, 0);
        lcd.print("Time:");
        lcd.print(millis() / 1000);
        lcd.print("s ");
    }

    // 2. Проверка флага сканирования (установлен внешним прерыванием)
    if (scanFlag && !keyProcessed) {
        // Временно отключаем прерывание для обработки
        detachInterrupt(digitalPinToInterrupt(KEY_INT_PIN));
        
        char pressedKey = scanKeyboard();

        if (pressedKey != 0) {
            lcd.setCursor(0, 0);
            lcd.print("Key: ");
            lcd.print(pressedKey);
            lcd.print("           ");
            
            Serial.print("Key Press Triggered Scan: ");
            Serial.println(pressedKey);

            // Дополнительная логика: сброс счетчика по нажатию 'A'
            if (pressedKey == 'A') {
                timerCount = 0;
                lcd.setCursor(7, 1);
                lcd.print("0   ");
            } else if (pressedKey == 'B') {
                // Остановка/запуск таймера по нажатию 'B'
                if (TIMSK1 & (1 << OCIE1A)) {
                    TIMSK1 &= ~(1 << OCIE1A); // Останавливаем таймер
                    lcd.setCursor(0, 0);
                    lcd.print("Timer STOPPED    ");
                } else {
                    TIMSK1 |= (1 << OCIE1A); // Запускаем таймер
                    lcd.setCursor(0, 0);
                    lcd.print("Timer RUNNING    ");
                }
            }
            
            lastKey = pressedKey;
        } else {
            lcd.setCursor(0, 0);
            lcd.print("Key Released?    ");
        }
        
        scanFlag = false;
        keyProcessed = true;
        
        // Переподключаем прерывание после обработки
        attachInterrupt(digitalPinToInterrupt(KEY_INT_PIN), startScan, FALLING);
    }
}