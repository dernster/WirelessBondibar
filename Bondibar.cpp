#include "Bondibar.h"

SINGLETON_CPP(Bondibar)

Bondibar::Bondibar(){
  SPI.begin();
  singleton(Configuration)->addObserver(this);
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

