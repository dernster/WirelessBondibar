#pragma once
#include "utils.h"
#include "Arduino.h"
#include "Configuration.h"

#define EXPIRATION_PERIOD (configuration->ClockSync->expirationPeriod)
#define CALIBRATE_PERIOD (24*6)

class TimeClock: public ConfigurationObserver{
SINGLETON_H(TimeClock)
public:
  Configuration* configuration;
  void setup(){
    minimumOffset = 2147483647;
    multiplier = 1;
    increment = 0;
    firstTime = true;
    correction = 0;
    cumulativeMean = 0;
    packets = 0;
    lastMean = 0;
    successCount = 0;
    isCalibrated = false;
    configuration = singleton(Configuration);

  }

  bool isCalibrated;
  long correction;
  unsigned long minimumOffset = 2147483647;
  unsigned long multiplier = 1;
  int increment = 0;
  bool firstTime = true;


  double lastMean;
  double cumulativeMean;
  unsigned long packets;
  int successCount;
  double updateMean(long offset){
    packets++;
    if (packets == 1){
      cumulativeMean = offset;
    }else{
      double i = packets;
      double x = offset;
      cumulativeMean = ((i-1)*cumulativeMean)/i + x/i;
    }
    return cumulativeMean;
  }

  void resetMeanCalibrate(){
    lastMean = 0;
    packets = 0;
    successCount = 0;
    cumulativeMean = 0;
  }

  bool inCalibratePeriod(){
    if (successCount >= CALIBRATE_PERIOD)
      return false;

    bool success = abs(cumulativeMean - lastMean) < 0.2;
    if (success){
      successCount++;
    }else{
      successCount = 0;
    }
    return (successCount <= CALIBRATE_PERIOD);
  }

  bool addServerOffsetSample(long serverOffset, bool expirationNeeded, unsigned long serverTime, unsigned long raw, byte seq){
    static int count = 0;
    unsigned long rawtime = rawTime();
    unsigned long currentTime = time();
    static bool hasEverExpired = false;
    bool selected = false;

    lastMean = cumulativeMean;
    double mean = updateMean(serverOffset);
    if (inCalibratePeriod()){
      if (hasEverExpired){
        Serial.printf("%i\t%i\t%lu\t%lu\t%lu\t%s\t%i\n", serverOffset, selected,minimumOffset, serverTime, raw, String(mean).c_str(),seq);
        return true;
      } else {
        return false;
      }
    }
    /* remove outlier */
    if (abs(serverOffset - mean) > configuration->ClockSync->offsetSigma){
      Serial.printf("%i\t%i\t%lu\t%lu\t%lu\t%s\t%i\n", serverOffset, selected,minimumOffset, serverTime, raw, String(mean).c_str(),seq);
      return true;
    }

    // if (expirationNeeded) {
    //   if (!count) {
    //     count = 1;
    //     Serial.printf("Expiration needed at: %lu\n", currentTime);
    //   }
    //   minimumOffset = 2147483647;
    // } else {
    //   count = 0;
    // }

    if (!firstTime) {
      if (currentTime >= multiplier*EXPIRATION_PERIOD) {
        multiplier++;
        // minimumOffset = expirationNeeded ? 2147483647 : ((unsigned long)(abs(correction) + pow(2,increment)));
        minimumOffset = ((unsigned long)(abs(correction) + pow(2,increment)));
        // Serial.printf("Expiration -> minOff %i\n", minimumOffset);
        increment++;
        resetMeanCalibrate();
        hasEverExpired = true;
      }
    } else {
      correction = serverOffset;
      firstTime = false;
      multiplier = ((double)time()/(double)EXPIRATION_PERIOD) + 1;
      Serial.println("serverOffset\tselected\tminimumOffset\tserverTime\trawTime\tmean\tseq");
    }

    if (((unsigned long)abs(serverOffset)) <= minimumOffset){
      // Serial.printf("Setting offset %i\n", serverOffset);
      isCalibrated = true;
      minimumOffset = (unsigned long)abs(serverOffset);
      correction = serverOffset;
      increment = 0;
      selected = true;
    }

    Serial.printf("%i\t%i\t%lu\t%lu\t%lu\t%s\t%i\n", serverOffset, selected,minimumOffset, serverTime, raw, String(mean).c_str(),seq);
    return true;
    // Serial.printf("serverOffset=%i\tminimumOffset=%i\tcorrection=%i\tmultiplier=%lu\ttime=%lu\n================\n",serverOffset,minimumOffset, correction, multiplier, time());
  }

  TimeClock(){
    setup();
  }

  unsigned long time(){
    bool isPositive = correction >= 0;
    unsigned long corr = (unsigned long) abs(correction);
    unsigned long milis = rawTime();
    return (unsigned long)(isPositive ? (milis + corr) : (milis - corr));
  }

  unsigned long rawTime(){
    return micros() / 1000;
  }

  void configurationChanged(){
    Serial.println("TimeClock::configurationChanged()");
    setup();
  }

};
