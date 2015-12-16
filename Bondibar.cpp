#include "Bondibar.h"

BondibarManager::BondibarManager(){
  SPI.begin();
}

void BondibarManager::sendData(byte* data, int len){
  SPI.writeBytes(data,len);
}

BondibarManager Bondibar;

