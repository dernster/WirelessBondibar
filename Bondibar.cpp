#include "Bondibar.h"

BondibarManager::BondibarManager(){
  SPI.begin();
}

void BondibarManager::sendData(byte* data, int len){
  SPI.writeBytes(data,len);
}

void BondibarManager::sendData(byte* data, int offset, int len){
  SPI.writeBytes(data + offset,len);
}

BondibarManager Bondibar;

