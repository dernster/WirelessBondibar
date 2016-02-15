#include "Bondibar.h"
#include "utils.h"

SINGLETON_CPP(Bondibar)
//
//void shiftOutFast(byte data)
//{
//  byte i = 8;
//  do{
//    GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1 << CLOCK);
//    if(data & 0x80)
//      GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << DATA);
//    else
//      GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1 << DATA);
//    GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << CLOCK);
//    data <<= 1;
//  }while(--i);
//  return;
//}

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

void Bondibar::sendData(byte* data, int len){
  SPI.writeBytes(data,len);
}

void Bondibar::sendData(byte* data, int offset, int len){
  SPI.writeBytes(data + offset,len);
//  for(int i = 0; i < len; i++)
//    shiftOutFast(data[offset + i]);
}

void Bondibar::configurationChanged(){
  Serial.println("BondibarManager::configurationChanged()");
}

