#include "Bondibar.h"
#include "utils.h"

SINGLETON_CPP(Bondibar)

#define LEN (24*2)

Bondibar::Bondibar(){
  SPI.begin();
  singleton(Configuration)->addObserver(this);

  byte red[LEN] = {0};
  byte blue[LEN] = {0};
  byte green[LEN] = {0};
  byte black[LEN] = {0};

  for(int i = 0; i < LEN; i++){
    if (i % 3 == 0)
      red[i] = 255;

    if (i % 3 == 1)
      blue[i] = 255;

    if (i % 3 == 2)
      green[i] = 255;
  }

  sendData(red,LEN);
  delay(500);
  sendData(green,LEN);
  delay(500);
  sendData(blue,LEN);
  delay(500);
  sendData(black,LEN);
}

void Bondibar::turnOffLights(){
  int size = singleton(Configuration)->Device->managedPixelsQty*3;
  byte* black = new byte[size];
  for(int i = 0; i < size; i++){
    black[i] = 0;
  }
  sendData(black, size);
  delete [] black;
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
