const int R=13,Y=12,G=11,BTN=2,EMERG=3;
enum St{GREEN,YELLOW,RED,RED_BLINK,WARNING,NIGHT}; 
St st;
unsigned long t0,dur,lastBtn=0;
bool ped=0,night=0,btn=0,prevEmerg=0;
int blinkCnt=0;

void leds(int r,int y,int g){digitalWrite(R,r);digitalWrite(Y,y);digitalWrite(G,g);}

void next(St s,unsigned long d){
  st=s; t0=millis(); dur=d; leds(0,0,0);
  switch(s){
    case GREEN: leds(0,0,1); break;
    case YELLOW: leds(0,1,0); break;
    case RED: leds(1,0,0); break;
    case RED_BLINK: blinkCnt=6; break; // 3 мигания по 500мс
  }
}

void setup(){
  pinMode(R,1); pinMode(Y,1); pinMode(G,1);
  pinMode(BTN,2); pinMode(EMERG,2);
  next(GREEN,10000);
}

void loop(){
  bool p=!digitalRead(BTN);
  if(p&&!btn&&millis()-lastBtn>50){ btn=1; lastBtn=millis(); }
  if(!p&&btn){ btn=0; unsigned long held=millis()-lastBtn;
    if(held>2000) night=!night; 
    else if(held>50) ped=1; 
  }

  bool emerg=!digitalRead(EMERG);
  if(emerg){ if(st!=WARNING) next(WARNING,500); if(millis()-t0>dur){ digitalWrite(Y,!digitalRead(Y)); t0=millis(); } return; }
  if(!emerg&&prevEmerg) next(GREEN,10000);
  prevEmerg=emerg;

  if(night){ if(st!=NIGHT) next(NIGHT,1000); if(millis()-t0>dur){ digitalWrite(Y,!digitalRead(Y)); t0=millis(); } return; }

  // --- Обработка пешехода ---  
  if(ped && st==GREEN){ // короткое нажатие на Green → RED_BLINK
    next(RED_BLINK,500);
    ped=0;
  }

  if(millis()-t0>dur) switch(st){
    case GREEN:  next(YELLOW,3000); break;
    case YELLOW: next(RED,10000); break;
    case RED:    next(GREEN,10000); break;
    case RED_BLINK:
      digitalWrite(R,!digitalRead(R));
      blinkCnt--; t0=millis();
      if(blinkCnt<=0) next(GREEN,10000);
      break;
  }
}