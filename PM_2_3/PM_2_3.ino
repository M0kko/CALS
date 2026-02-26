boolean statuslamp; // состояние лампы: true - включено, false - выключено

void setup() {
   pinMode(12,OUTPUT); // пин 12 со светодиодом будет выходом (англ. «output»)
   pinMode(A0,INPUT); // к аналоговому входу A0 подключим датчик (англ. «intput»)
   statuslamp=false; // начальное состояние - лампа выключена
   Serial.begin(9600); // подключаем монитор порта
}

void loop() {
   Serial.println (analogRead(A0)); // выводим значение датчика на монитор
  
   if(analogRead(A0)>60) {
      statuslamp=!statuslamp; // меняем статус лампы при регистрации хлопка
      digitalWrite(12,statuslamp); // переключаем светодиод на выходе 12
      delay(20); // задержка, "дребезга" хлопков
   }
}