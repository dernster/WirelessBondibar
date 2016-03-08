#pragma once
#include "utils.h"
#include "Arduino.h"

#define EXPIRATION_PERIOD (3*60*1000)

class TimeClock{
SINGLETON_H(TimeClock)
public:
  void setup(){
    minimumOffset = 2147483647;
    multiplier = 1;
    increment = 0;
    firstTime = true;
    correction = 0;
  }

  long correction;
  unsigned long minimumOffset = 2147483647;
  unsigned long multiplier = 1;
  int increment = 0;
  bool firstTime = true;

  void addServerOffsetSample(long serverOffset){
    static int count = 0;
    unsigned long rawtime = rawTime();
    unsigned long currentTime = time();
    
    if (count++ < 5)
      return;

    if (!firstTime) {
      if (currentTime >= multiplier*EXPIRATION_PERIOD) {
        multiplier++;
        minimumOffset = 2147483647;//(unsigned long)(abs(correction) + pow(2,increment));
        Serial.printf("Expiration -> minOff %i\n", minimumOffset);
        increment++;
      }
    } else {
      correction = serverOffset;
      firstTime = false;
      multiplier = ((double)time()/(double)EXPIRATION_PERIOD) + 1;
    }

    if (((unsigned long)abs(serverOffset)) < minimumOffset){
      Serial.printf("Setting offset %i\n", serverOffset);
      minimumOffset = (unsigned long)abs(serverOffset);
      correction = serverOffset;
      increment = 0;
    }

    Serial.printf("serverOffset=%i\tminimumOffset=%i\tcorrection=%i\tmultiplier=%lu\ttime=%lu\n================\n",serverOffset,minimumOffset, correction, multiplier, time());
  }

  TimeClock(){
    setup();
  }

  unsigned long time(){
    bool isPositive = correction >= 0;
    unsigned long corr = (unsigned long) abs(correction);
    unsigned long milis = millis();
    // if ((!isPositive) && (milis < corr)){
    //   Serial.println("ERROR INSANO LOCO!!!!!!!!!!!!!!!!!!!!!!");
    //   while(true){};
    // }
    // if ((isPositive) && (milis + corr < milis)){
    //   Serial.println("ERROR INSANO LOCO 2!!!!!!!!!!!!!!!!!!!!!!");
    //   while(true){};
    // }
    return (unsigned long)(isPositive ? (milis + corr) : (milis - corr));
  }

  unsigned long rawTime(){
    return millis();
  }

};
