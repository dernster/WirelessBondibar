#pragma once
#include "utils.h"
#include "Arduino.h"
#include "Configuration.h"
#include "DataTypes.h"

#define EXPIRATION_PERIOD (configuration->ClockSync->expirationPeriod)
#define PLAYBACK_TIME_DELAY (configuration->Streaming->playbackTimeDelay)
#define OFFSET_MEAN_CALIBRATION_CONSECUTIVE_PACKETS (configuration->ClockSync->offsetMeanCalibrationConsecutivePackets)
#define OFFSET_MEAN_CALIBRATION_DERIVATIVE_THRESHOLD (configuration->ClockSync->offsetMeanCalibrationDerivativeThreshold)
#define FIRST_PACKETS_IGNORE_QTY (configuration->ClockSync->firstPacketsIgnoreQty)

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
    totalPackets = 0;
    wasCalibratedAtLeastOneTime = false;
    resetMeanCalibrate();
    configuration = singleton(Configuration);

  }

  bool wasCalibratedAtLeastOneTime;
  long correction;
  unsigned long minimumOffset = 2147483647;
  unsigned long multiplier = 1;
  int increment = 0;
  bool firstTime = true;
  unsigned int totalPackets;


  OffsetSample lastMean;
  OffsetSample cumulativeMean;
  unsigned long packets;
  int successCount;
  unsigned long calibrated = 0;
  double updateMean(OffsetSample offsetSample){
    lastMean = cumulativeMean;
    packets++;
    if (packets == 1){
      cumulativeMean = offsetSample;
    }else{
      double i = packets;
      double x = offsetSample.sample;
      cumulativeMean.sample = ((i-1)*cumulativeMean.sample)/i + x/i;
      cumulativeMean.timestamp = offsetSample.timestamp;
    }
    return cumulativeMean.sample;
  }

  void resetMeanCalibrate(){
    lastMean.sample = lastMean.timestamp = 0;
    cumulativeMean.sample = cumulativeMean.timestamp = 0;
    packets = 0;
    successCount = 0;
    calibrated = 0;
  }

  bool inCalibratePeriod(){
    if (successCount >= OFFSET_MEAN_CALIBRATION_CONSECUTIVE_PACKETS)
      return false;
    if (packets == 1)
      return true;

    double deltaX = cumulativeMean.sample - lastMean.sample;
    double deltaT = cumulativeMean.timestamp - lastMean.timestamp;
    bool success = abs(deltaX/deltaT) < OFFSET_MEAN_CALIBRATION_DERIVATIVE_THRESHOLD;
    if (success){
      successCount++;
    }else{
      successCount = 0;
    }
    return (successCount <= OFFSET_MEAN_CALIBRATION_CONSECUTIVE_PACKETS);
  }

  bool updateServerOffset(Frame* frame){

    if (totalPackets < FIRST_PACKETS_IGNORE_QTY){
      totalPackets++;
      return false;
    }

    long serverOffset = (frame->pt -PLAYBACK_TIME_DELAY) - frame->arriveTime;
    byte seq = frame->seq;
    unsigned long rawtime = rawTime();
    unsigned long currentTime = time();
    static bool hasEverExpired = false;
    bool selected = false;

    OffsetSample offset(serverOffset,frame->arriveTime);
    double mean = updateMean(offset);
    if (inCalibratePeriod()){
      if (hasEverExpired){
        DEBUG(Serial.printf("%i\t%i\t%lu\t%s\t%i\t%i\n", serverOffset, selected,minimumOffset, String(mean).c_str(),seq, calibrated));
        return true;
      } else {
        return false;
      }
    }
    calibrated = serverOffset;
    /* remove outlier */
    if (abs(serverOffset - mean) > configuration->ClockSync->offsetSigma){
      if (wasCalibratedAtLeastOneTime)
        DEBUG(Serial.printf("%i\t%i\t%lu\t%s\t%i\t%i\n", serverOffset, selected,minimumOffset, String(mean).c_str(),seq, calibrated));
      return wasCalibratedAtLeastOneTime;
    }

    if (!firstTime) {
      if (currentTime >= multiplier*EXPIRATION_PERIOD) {
        multiplier++;
        minimumOffset = ((unsigned long)(abs(correction) + pow(2,increment)));
        DEBUG(Serial.printf("Expiration -> minOff %i\n", minimumOffset));
        increment++;
        resetMeanCalibrate();
        hasEverExpired = true;
      }
    } else {
      correction = serverOffset;
      firstTime = false;
      multiplier = ((double)time()/(double)EXPIRATION_PERIOD) + 1;
      DEBUG(Serial.println("serverOffset\tselected\tminimumOffset\tmean\tseq\tcalibrated"));
    }

    if (((unsigned long)abs(serverOffset)) <= minimumOffset){
      DEBUG(Serial.printf("Setting offset %i\n", serverOffset));
      wasCalibratedAtLeastOneTime = true;
      minimumOffset = (unsigned long)abs(serverOffset);
      correction = serverOffset;
      increment = 0;
      selected = true;
    }

    DEBUG(Serial.printf("%i\t%i\t%lu\t%s\t%i\t%i\n", serverOffset, selected,minimumOffset, String(mean).c_str(),seq, calibrated));
    return true;
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
    return millis();
  }

  void configurationChanged(){
    Serial.println("TimeClock::configurationChanged()");
    setup();
  }

};
