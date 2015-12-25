#include "Bondibar.h"

BondibarManager::BondibarManager(){
  SPI.begin();
  singleton(Configuration)->addObserver(this);
}

void BondibarManager::sendData(byte* data, int len){
  SPI.writeBytes(data,len);
}

void BondibarManager::sendData(byte* data, int offset, int len){
  SPI.writeBytes(data + offset,len);
}

void BondibarManager::configurationChanged(){
  Serial.println("BondibarManager::configurationChanged()");
}

BondibarManager Bondibar;

