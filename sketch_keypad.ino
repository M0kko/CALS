// Выводы для клавиатуры 4x4
const int rowPins[4] = {7,6,5,4}; // Строки (Row)
const int colPins[4] = {11,10,9,8}; // Столбцы (Col)

char keys[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

volatile byte currentRow = 0;

void setupKeyboardPins() {
    // Настройка строк как выходы (управляем ими)
    for(int i = 0; i < 4; i++) {
        pinMode(rowPins[i], OUTPUT);
        digitalWrite(rowPins[i], HIGH); // Изначально все строки отключены (HIGH)
    }
    // Настройка столбцов как входы (читаем их)
    for(int i = 0; i < 4; i++) {
        pinMode(colPins[i], INPUT_PULLUP); // Используем внутреннюю подтяжку
    }
}

void setup() {
    Serial.begin(9600);
    setupKeyboardPins();
    
    // Настройка Timer0 для генерации прерывания каждые 1 мс
    noInterrupts();
    
    TCCR0A = 0; // Сбрасываем регистры
    TCCR0B = 0;
    TCNT0 = 0;
    
    // Устанавливаем предделитель 64
    TCCR0B |= (1 << CS01) | (1 << CS00);
    
    // Включаем режим CTC
    TCCR0A |= (1 << WGM01);
    
    // Устанавливаем регистр сравнения для 1 кГц (1 мс)
    OCR0A = 249; // = (16*10^6) / (64 * 1000) - 1
    
    // Разрешаем прерывание по сравнению
    TIMSK0 |= (1 << OCIE0A);
    
    interrupts();
    
    Serial.println("Задание 3: Сканирование клавиатуры по таймеру");
    Serial.println("Готов к чтению клавиш...");
}

void loop() {
    
}

// ISR для сканирования (вызывается Timer0 каждые 1 мс)
ISR(TIMER0_COMPA_vect) {
    // 1. Отключаем предыдущую строку
    digitalWrite(rowPins[currentRow], HIGH);

    // 2. Переходим к следующей строке (циклически)
    currentRow = (currentRow + 1) % 4;

    // 3. Активируем новую строку (подаем LOW)
    digitalWrite(rowPins[currentRow], LOW);

    // 4. Читаем столбцы и проверяем нажатия
    for (int col = 0; col < 4; col++) {
        if (digitalRead(colPins[col]) == LOW) {
            // Обнаружено нажатие
            Serial.print("Нажата: ");
            Serial.println(keys[currentRow][col]);
            break; // Выходим после первого найденного нажатия в строке
        }
    }
}