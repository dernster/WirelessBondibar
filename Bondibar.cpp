#include "Bondibar.h"
#include "utils.h"

SINGLETON_CPP(Bondibar)

Bondibar::Bondibar(){
  SPI.begin();
  singleton(Configuration)->addObserver(this);

  byte red[24] = {0};
  byte blue[24] = {0};
  byte green[24] = {0};
  
  for(int i = 0; i < 24; i++){
    if (i % 3 == 0)
      red[i] = 255;

    if (i % 3 == 1)
      blue[i] = 255;

    if (i % 3 == 2)
      green[i] = 255;
  }
  
  sendData(red,24);
  flashLed(50,0,1);
  delay(500);
  sendData(green,24);
  flashLed(50,0,1);
  delay(500);
  sendData(blue,24);
  flashLed(50,0,1);
  delay(500);
}

void Bondibar::sendData(byte* data, int len){
  SPI.writeBytes(data,len);
}

void Bondibar::sendData(byte* data, int offset, int len){
  SPI.writeBytes(data + offset,len);
}

void Bondibar::configurationChanged(){
  Serial.println("BondibarManager::configurationChanged()");
}

