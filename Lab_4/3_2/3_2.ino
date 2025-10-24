#include "LedControl.h"
#include "binary.h"
/*
DIN подключен к пину 11
CLK подключен к пину 13
CS подключен к пину 8
*/
LedControl matr=LedControl(11,13,10,1);
// Счастливый смайл
byte hf[8]= {B00111100,B01000010,B10011001,B10100101,B10000001,B10100101,B01000010,B00111100};
// Нейтральный смайл
byte nf[8]= {B00111100,B01000010,B10000001,B10111101,B10000001,B10100101,B01000010,B00111100};
// Печальный смайл
byte sf[8]= {B00111100,B01000010,B10100101,B10011001,B10000001,B10100101,B01000010,B00111100};

void setup() {
  matr.shutdown(0,false); //Включаем светодиодную матрицу
  matr.setIntensity(0,8); // Установка яркости на среднее значение
  matr.clearDisplay(0); // Очистка матрицы
  Serial.begin(9600);
}

void loop(){
  //Вывод счастливого смайла
  matr.setRow(0,0,hf[0]);
  matr.setRow(0,1,hf[1]);
  matr.setRow(0,2,hf[2]);
  matr.setRow(0,3,hf[3]);
  matr.setRow(0,4,hf[4]);
  matr.setRow(0,5,hf[5]);
  matr.setRow(0,6,hf[6]);
  matr.setRow(0,7,hf[7]);
  delay(1000); //задержка 1 с
  //Вывод нейтрального смайла
  matr.setRow(0,0,nf[0]);
  matr.setRow(0,1,nf[1]);
  matr.setRow(0,2,nf[2]);
  matr.setRow(0,3,nf[3]);
  matr.setRow(0,4,nf[4]);
  matr.setRow(0,5,nf[5]);
  matr.setRow(0,6,nf[6]);
  matr.setRow(0,7,nf[7]);
  delay(1000); //задержка 1 с
  //Вывод печального смайла
  matr.setRow(0,0,sf[0]);
  matr.setRow(0,1,sf[1]);
  matr.setRow(0,2,sf[2]);
  matr.setRow(0,3,sf[3]);
  matr.setRow(0,4,sf[4]);
  matr.setRow(0,5,sf[5]);
  matr.setRow(0,6,sf[6]);
  matr.setRow(0,7,sf[7]);
  delay(1000); //задержка 1 с
}