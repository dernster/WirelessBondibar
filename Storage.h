#pragma once
#include <EEPROM.h>
#include "utils.h"

class Storage{
SINGLETON_H(Storage)
public:
  byte readDeviceID();
  void readSSIDAndPassword(String& ssid, String& password);
  void setSSIDAndPassword(String ssid, String password);
};

