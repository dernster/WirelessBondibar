#include <SPI.h>
#include "Configuration.h"

#define CLK_PIN 14 // D5, GPIO14
#define DATA_PIN 13 // D7, GPIO13

class Bondibar : public ConfigurationObserver{
SINGLETON_H(Bondibar)
public:
  Bondibar();
  void sendData(byte* data, int len);
  void sendData(byte* data, int offset, int len);

  void configurationChanged();
};

