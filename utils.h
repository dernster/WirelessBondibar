#include "Arduino.h"
#include <ESP8266WiFi.h>
#include "Configuration.h"
#include <vector>
#include "Dictionary.h"
using namespace std;



#define U32_TIME_GREATER_THAN(a, b) (((long)((a) - (b))) > 0)
#define DEBUG(x)

#define time_r long

#define LED 2

#define LOOP_UNTIL(ms)\
long int start = millis();\
while((millis() - start) < ms)

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


void flashLed(int onMs, int offMs, int times);
void printWifiStatus();
String ipToString(IPAddress ip);
IPAddress stringToIP(String ipStr);
void connectToWifi();
vector<String> splitString(String string, char delimiter);
Dictionary parseParameters(String params);
byte* copyBuffer(byte* src,int len);
bool msIsMultiple(unsigned long time, unsigned long multiple);
