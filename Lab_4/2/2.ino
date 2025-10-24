#define LATCH_DIO 4  // ST_CP
#define CLK_DIO   7  // SH_CP
#define DATA_DIO  8  // DS

// Коды цифр для индикатора с общим анодом
const byte seg_digits[] = {
  0xC0, // 0
  0xF9, // 1
  0xA4, // 2
  0xB0, // 3
  0x99, // 4
  0x92, // 5
  0x82, // 6
  0xF8, // 7
  0x80, // 8
  0x90  // 9
};

// Коды включения разрядов (активный 0)
const byte seg_pos[] = {0xF1, 0xF2, 0xF4, 0xF8};

unsigned long prevMillis = 0;
unsigned int refreshDelay = 3;  // задержка между разрядами (мс)
unsigned long counter = 0;
unsigned long counterUpdate = 0;

void setup() {
  pinMode(LATCH_DIO, OUTPUT);
  pinMode(CLK_DIO, OUTPUT);
  pinMode(DATA_DIO, OUTPUT);
}

void loop() {
  unsigned long now = millis();

  // Каждые 300 мс увеличиваем счётчик
  if (now - counterUpdate >= 300) {
    counterUpdate = now;
    counter++;
    if (counter > 9999) counter = 0;
  }

  // Отображаем цифры по очереди
  for (byte i = 0; i < 4; i++) {
    byte digit;
    switch (i) {
      case 0: digit = (counter / 1000) % 10; break;
      case 1: digit = (counter / 100) % 10; break;
      case 2: digit = (counter / 10) % 10; break;
      case 3: digit = counter % 10; break;
    }

    digitalWrite(LATCH_DIO, LOW);
    shiftOut(DATA_DIO, CLK_DIO, MSBFIRST, seg_digits[digit]);
    shiftOut(DATA_DIO, CLK_DIO, MSBFIRST, seg_pos[i]);
    digitalWrite(LATCH_DIO, HIGH);

    delay(refreshDelay);  // небольшая пауза
  }
}
