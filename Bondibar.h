#include <SPI.h>

#define CLK_PIN 14 // D5, GPIO14
#define DATA_PIN 13 // D7, GPIO13

class BondibarManager{
public:
  BondibarManager();
  void sendData(byte* data, int len);
};

extern BondibarManager Bondibar;

