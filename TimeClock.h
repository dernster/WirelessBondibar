#pragma once
#include "utils.h"
#include "Arduino.h"

#define EXPIRATION_PERIOD (30*1000)

class TimeClock{
SINGLETON_H(TimeClock)
public:
  void reset(){
    minimumOffset = 2147483647;
    multiplier = 1;
    increment = 0;
    firstTime = true;
    correction = 0;
  }

  long correction;

  unsigned long minimumOffset = 2147483647;
  int multiplier = 1;
  int increment = 0;
  bool firstTime = true;

  void addServerOffsetSample(long serverOffset){
    unsigned long rawtime = rawTime();
    unsigned long currentTime = rawtime + correction;

    if (!firstTime) {
      if (currentTime >= multiplier*EXPIRATION_PERIOD) {
        multiplier++;
        minimumOffset = 2147483647;//(unsigned long)(abs(correction) + pow(2,increment));
        Serial.printf("Expiration -> minOff %i\n", minimumOffset);
        increment++;
      }
    } else {
      firstTime = false;
      multiplier = ((double)time()/(double)EXPIRATION_PERIOD) + 1;
    }

    if (((unsigned long)abs(serverOffset)) < minimumOffset){
      Serial.printf("Setting offset %i; rawTime=%i; time=%i\n", serverOffset, rawtime, currentTime);
      minimumOffset = (unsigned long)abs(serverOffset);
      correction = serverOffset;
      increment = 0;
    }
  }

  TimeClock(){
    correction = 0;
  }

  unsigned long time(){
    bool isPositive = correction >= 0;
    unsigned long corr = (unsigned long) abs(correction);

    return (unsigned long)(isPositive ? (millis() + corr) : (millis() - corr));
  }

  unsigned long rawTime(){
    return millis();
  }

};
