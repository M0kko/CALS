volatile bool fastMode = false; 
const int ledPin = 13;          
const int buttonPin = 2;        

unsigned long previousMillis = 0;
unsigned long interval = 1000;  // Интервал мигания 1 сек
bool ledState = false;          // Текущее состояние светодиода

volatile bool lastButtonState = HIGH;
volatile bool buttonPressed = false;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); // Подтяжка к питанию
  
  // Включаем светодиод при старте
  digitalWrite(ledPin, HIGH);
  ledState = true;
  
  // Инициализируем начальное состояние кнопки
  lastButtonState = digitalRead(buttonPin);
  
  // Подключаем прерывание на пине 2 по режиму CHANGE
  attachInterrupt(digitalPinToInterrupt(buttonPin), changeMode, CHANGE);
  
  Serial.begin(9600);
  Serial.println("Система запущена. Исходная частота: 1 Гц");
  Serial.println("Светодиод включен при старте");
  Serial.println("Режим CHANGE - срабатывает при нажатии и отпускании");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Управление миганием светодиода
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    ledState = !ledState; // Инвертируем состояние
    digitalWrite(ledPin, ledState); // Устанавливаем состояние
    
    // Вывод информации для отладки
    if (ledState) {
      Serial.println("Светодиод ВКЛ");
    } else {
      Serial.println("Светодиод ВЫКЛ");
    }
  }
  
  // Обработка нажатия кнопки (только при переходе с HIGH на LOW)
  if (buttonPressed) {
    buttonPressed = false;
    handleButtonPress();
  }
}

// Обработчик прерывания - определяет состояние кнопки
void changeMode() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  
  // Защита от дребезга - игнорируем события чаще 50 мс
  if (interruptTime - lastInterruptTime > 50) {
    bool currentButtonState = digitalRead(buttonPin);
    
    // Реагируем только на нажатие (переход с HIGH на LOW)
    if (lastButtonState == HIGH && currentButtonState == LOW) {
      buttonPressed = true;
    }
    
    lastButtonState = currentButtonState;
  }
  lastInterruptTime = interruptTime;
}

// Обработка нажатия кнопки - меняет режим мигания
void handleButtonPress() {
  fastMode = !fastMode; // Меняем режим
  
  if (fastMode) {
    interval = 200; // 5 раз в секунду (1000/5 = 200 мс)
    Serial.println(">>> Переключено в быстрый режим: 5 Гц");
  } else {
    interval = 1000; // 1 раз в секунду
    Serial.println(">>> Переключено в обычный режим: 1 Гц");
  }
  
  // Сбрасываем таймер для немедленного применения нового интервала
  previousMillis = millis();
}