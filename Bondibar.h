#include <SPI.h>
#include "Configuration.h"

#define CLK_PIN 14 // D5, GPIO14
#define DATA_PIN 13 // D7, GPIO13

class BondibarManager : public ConfigurationObserver{
public:
  BondibarManager();
  void sendData(byte* data, int len);
  void sendData(byte* data, int offset, int len);

  void configurationChanged();
};

extern BondibarManager Bondibar;

