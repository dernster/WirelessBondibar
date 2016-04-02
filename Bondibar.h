#include <SPI.h>
#include "Configuration.h"

#define CLK_PIN 14 // D5, GPIO14
#define DATA_PIN 13 // D7, GPIO13

enum ColorOrderType { RGB, RBG, BGR, BRG, GRB, GBR };

class Bondibar : public ConfigurationObserver{
SINGLETON_H(Bondibar)
public:
  Bondibar();
  void setup();

  void sendData(byte* data, int len);
  void turnOffLights();

  void configurationChanged();

private:
  vector<ColorOrderType> colorOrderList;
};
