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

  time_t minimumOffset = 2147483647;
  int multiplier = 1;
  time_t increment = 0;
  bool firstTime = true;
  void addServerOffsetSample(time_t serverOffset){
    time_t currentTime = time();

    if (!firstTime && (currentTime >= multiplier*EXPIRATION_PERIOD)){
      multiplier++;
      minimumOffset = abs(correction)  + (long)pow(2,increment);
      Serial.printf("Expiration -> minOff %i\n", minimumOffset);
      increment++;
    }

    if (abs(serverOffset) < minimumOffset){
      Serial.printf("Setting offset %i\n", serverOffset);
      minimumOffset = abs(serverOffset);
      correction = serverOffset;
      increment = 0;
    }

    if (firstTime) {
      firstTime = false;
      multiplier = ((double)time()/(double)EXPIRATION_PERIOD) + 1;
    }
  }

  TimeClock(){
    correction = 0;
  }

  time_r time(){
    return ((time_r)millis()) + correction;
  }

  time_r rawTime(){
    return millis();
  }

};
