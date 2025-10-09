const int ledPin = 13; // Светодиод на пине 13

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  
  // Настройка Timer1 для прерывания с частотой 100 Гц
  noInterrupts(); // Отключаем прерывания на время настройки
  
  TCCR1A = 0;    // Сбрасываем регистр A
  TCCR1B = 0;    // Сбрасываем регистр B
  TCNT1 = 0;     // Сбрасываем счетчик
  
  // Установка режима CTC (Clear Timer on Compare Match)
  TCCR1B |= (1 << WGM12);
  
  // Установка предделителя 64
  TCCR1B |= (1 << CS11) | (1 << CS10);
  
  // Установка значения сравнения для частоты 100 Гц
  // Расчет: 16,000,000 / (64 * 100) - 1 = 2499
  OCR1A = 2499;
  
  // Разрешаем прерывание по совпадению
  TIMSK1 |= (1 << OCIE1A);
  
  interrupts(); // Разрешаем прерывания
  
  Serial.println("Timer1 настроен на 100 Гц");
}

void loop() {
  // Основной цикл может выполнять другие задачи
  delay(1000);
  Serial.println("Основной цикл работает...");
}

// Обработчик прерывания таймера Timer1
ISR(TIMER1_COMPA_vect) {
  digitalWrite(ledPin, !digitalRead(ledPin)); // Инвертируем светодиод
}