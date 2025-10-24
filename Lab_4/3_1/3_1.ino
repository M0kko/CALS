#include <LedControl.h>
// DIN = 11 (MOSI), CLK = 13 (SCK), CS = 10
LedControl lc = LedControl(11, 13, 10, 1);
void setup() {
  lc.shutdown(0, false); 
  lc.setIntensity(0, 8); 
  lc.clearDisplay(0);
  // Пример: диагональ
  for (int i=0;i<8;i++) {
    lc.setLed(0, i, i, true);
  }
}
void loop(){ }