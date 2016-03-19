#pragma once
#include "utils.h"
#include "Arduino.h"
#include "Configuration.h"
#include "DataTypes.h"

class ServerOffsetCumulativeMean{
private:
  Configuration* conf;
  Sample<double> lastCumulativeMean;
  Sample<double> cumulativeMean;
  unsigned long packets;
  int successCount;
  bool wasCalibratedAtLeastOnce;
public:
  ServerOffsetCumulativeMean();
  double updateMean(const Sample<long> serverOffsetSample);
  bool isCalibrated();
  long getMean();
  void reset();
  void expireCalculation();
};

class TimeClock: public ConfigurationObserver {
SINGLETON_H(TimeClock)
private:
  Configuration* configuration;
  ServerOffsetCumulativeMean serverOffsetCumulativeMean;
  long correction;
  bool synced;
  unsigned long minimumOffset;
  unsigned long expirationPeriodIndex;
  int offsetExpirationExponent;
  unsigned int firstInvalidPacketsQty;

  void configurationChanged();
public:
  TimeClock();
  void setup();
  bool isSynced();
  bool updateServerOffset(Frame* frame);
  unsigned long time();
  unsigned long rawTime();
};
