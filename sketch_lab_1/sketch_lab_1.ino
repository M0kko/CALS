void setup() {
  Serial.begin(9600);

  // Настраиваем A0–A2 (PINC 0,1,2) как входы, остальные тоже оставим входами
  DDRC = 0b00000000; // все входы

  // Настраиваем PB3–PB5 как выходы для светодиодов
  DDRB = 0b00111000; 
}

void loop() {
  // Сбрасываем все светодиоды
  PORTB = 0b00000000;

  // Проверяем кнопку A0
  if (PINC & 0b00000001) {
    Serial.println("Нажата кнопка A0");
    PORTB = 0b00100000; // PB5 → красный
    Serial.println("Загорелся КРАСНЫЙ → PORTB, бит 5");
  }

  // Проверяем кнопку A1
  if (PINC & 0b00000010) {
    Serial.println("Нажата кнопка A1");
    PORTB = 0b00010000; // PB4 → жёлтый
    Serial.println("Загорелся ЖЁЛТЫЙ → PORTB, бит 4");
  }

  // Проверяем кнопку A2
  if (PINC & 0b00000100) {
    Serial.println("Нажата кнопка A2");
    PORTB = 0b00001000; // PB3 → зелёный
    Serial.println("Загорелся ЗЕЛЁНЫЙ → PORTB, бит 3");
  }

  // Выводим состояние всех 6 аналоговых входов A0–A5
  Serial.print("Состояние PINC: ");
  for (int i = 5; i >= 0; i--) {
    Serial.print((PINC >> i) & 1);
  }
  Serial.println();

  delay(1000);
}