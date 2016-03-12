#pragma once
#include "utils.h"
#include "Arduino.h"

#define MAX_LONG (2147483647)

#define EXPIRATION_PERIOD (1*60*1000)
#define ARRIVED_COUNT (3)

class TimeClock{
SINGLETON_H(TimeClock)
public:
  void setup(){
    minimumOffset = MAX_LONG;
    multiplier = 1;
    increment = 0;
    firstTime = true;
    correction = 0;
    arrivedCount = 0;
    maxArrivedOffset = 0;
  }

  long correction;
  int arrivedCount = 0;
  long maxArrivedOffset = MAX_LONG;
  unsigned long minimumOffset = MAX_LONG;
  unsigned long multiplier = 1;
  int increment = 0;
  bool firstTime = true;

  void addServerOffsetSample(long serverOffset, bool expirationNeeded){
    static int count = 0;
    unsigned long rawtime = rawTime();
    unsigned long currentTime = time();

    if (expirationNeeded) {
      if (!count) {
        count = 1;
        Serial.printf("Expiration needed at: %lu\n", currentTime);
      }
      minimumOffset = MAX_LONG;
    } else {
      count = 0;
    }

    if (!firstTime) {
      if (currentTime >= multiplier*EXPIRATION_PERIOD) {
        multiplier++;
        arrivedCount = 0;
        minimumOffset = expirationNeeded ? MAX_LONG : ((unsigned long)(abs(correction) + pow(2,increment)));
        Serial.printf("Expiration -> minOff %i\n", minimumOffset);
        increment++;
      }
    } else {
      arrivedCount = 0;
      maxArrivedOffset = 0;
      correction = serverOffset;
      firstTime = false;
      multiplier = ((double)time()/(double)EXPIRATION_PERIOD) + 1;
    }

    if (((unsigned long)abs(serverOffset)) < minimumOffset){
      arrivedCount++;

      if (((unsigned long)abs(serverOffset)) > ((unsigned long)abs(maxArrivedOffset))) {
          maxArrivedOffset = serverOffset;
      }

      Serial.printf("Found %i offset\n", arrivedCount);
      if (arrivedCount >= ARRIVED_COUNT) {
        Serial.printf("--- Now synchronizing ---\n");
        // Serial.printf("Setting offset %i\n", serverOffset);
        minimumOffset = (unsigned long)abs(maxArrivedOffset);
        correction = maxArrivedOffset;
        increment = 0;
        arrivedCount = 0;
        maxArrivedOffset = 0;
      }
    }

    // Serial.printf("serverOffset=%i\tminimumOffset=%i\tcorrection=%i\tmultiplier=%lu\ttime=%lu\n================\n",serverOffset,minimumOffset, correction, multiplier, time());
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
