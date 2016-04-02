#include "Bondibar.h"
#include "utils.h"
#include "assert.h"

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

  SPI.writeBytes(red,LEN);
  delay(500);
  SPI.writeBytes(green,LEN);
  delay(500);
  SPI.writeBytes(blue,LEN);
  delay(500);
  SPI.writeBytes(black,LEN);
}

void Bondibar::setup() {
  vector<String> colorOrderListAux = splitString(singleton(Configuration)->Device->colorOrder, ',');
  for (int i = 0; i < colorOrderListAux.size(); i++){
    if (colorOrderListAux[i] == "RGB")
      colorOrderList.push_back(RGB);
    else if (colorOrderListAux[i] == "RBG")
      colorOrderList.push_back(RBG);
    else if (colorOrderListAux[i] == "BRG")
      colorOrderList.push_back(BRG);
    else if (colorOrderListAux[i] == "BGR")
      colorOrderList.push_back(BGR);
    else if (colorOrderListAux[i] == "GRB")
      colorOrderList.push_back(GRB);
    else if (colorOrderListAux[i] == "GBR")
      colorOrderList.push_back(GBR);
  }
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
  int i;
  for (i = 0; i < colorOrderList.size(); i++, data += 3) {
    switch (colorOrderList[i]) {
      case RGB:
        SPI.writeBytes(data, 3);
        break;
      case RBG:
        SPI.write(data[0]);
        SPI.write(data[2]);
        SPI.write(data[1]);
        break;
      case BGR:
        SPI.write(data[2]);
        SPI.write(data[1]);
        SPI.write(data[0]);
        break;
      case BRG:
        SPI.write(data[2]);
        SPI.write(data[0]);
        SPI.write(data[1]);
        break;
      case GRB:
        SPI.write(data[1]);
        SPI.write(data[0]);
        SPI.write(data[2]);
        break;
      case GBR:
        SPI.write(data[1]);
        SPI.write(data[2]);
        SPI.write(data[0]);
        break;
      default:
        break;
    }
  }
}

void Bondibar::configurationChanged(){
  Serial.println("BondibarManager::configurationChanged()");
}
