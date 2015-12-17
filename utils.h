#include "Arduino.h"
#include <ESP8266WiFi.h>
#include "Configuration.h" 

#define SINGLETON_H(ClassName)\
private:\
  static ClassName* instance;\
public:\
  static ClassName* getInstance();

#define SINGLETON_CPP(ClassName)\
ClassName* ClassName::instance = NULL;\
ClassName* ClassName::getInstance(){\
  if (instance == NULL){\
    instance = new ClassName();\
  }\
  return instance;\
}

#define singleton(ClassName) ClassName::getInstance()

void printWifiStatus();
String ipToString(IPAddress ip);
void connectToWifi();
