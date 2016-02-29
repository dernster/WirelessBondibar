#pragma once
#include "utils.h"
#include "Arduino.h"

#define EXPIRATION_PERIOD (30*1000)

class TimeClock{
SINGLETON_H(TimeClock)
public:
  long correction;

  time_t minimumOffset = 2147483647;
  int multiplier = 1;
  bool inExpirationPeriod = false;
  void addServerOffsetSample(time_t serverOffset){
    static long n = 0;
    time_t currentTime = time();

    if (n != 0 && !inExpirationPeriod && (currentTime >= multiplier*EXPIRATION_PERIOD)){
      inExpirationPeriod = true;
      multiplier++;
    }

    if (inExpirationPeriod){
      minimumOffset = (double)minimumOffset * 1.05;
      // Serial.printf("Incrementing minimum offset -> minimumOffset: %i\n", minimumOffset);
    }

    if (abs(serverOffset) < abs(minimumOffset)){
      // Serial.printf("Setting offset %i\n", serverOffset);
      minimumOffset = serverOffset;

      if (inExpirationPeriod){
        // Serial.println("Expiration perdiod finished!");
        inExpirationPeriod = false;
      }
    }

    if (!inExpirationPeriod){
      correction = minimumOffset;
    }

    if (n == 0){
      multiplier = (double)time()/(double)EXPIRATION_PERIOD + 1;
    }

    n++;
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
