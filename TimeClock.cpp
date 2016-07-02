#include "TimeClock.h"

SINGLETON_CPP(TimeClock)

/************************ ServerOffsetCumulativeMean **************************/

#define OFFSET_MEAN_CALIBRATION_CONSECUTIVE_PACKETS (conf->ClockSync->offsetMeanCalibrationConsecutivePackets)
#define OFFSET_MEAN_CALIBRATION_DERIVATIVE_THRESHOLD (conf->ClockSync->offsetMeanCalibrationDerivativeThreshold)

ServerOffsetCumulativeMean::ServerOffsetCumulativeMean(){
  reset();
}

double ServerOffsetCumulativeMean::updateMean(Sample<long> offsetSample){
  lastCumulativeMean = cumulativeMean;
  packets++;
  if (packets == 1){
    cumulativeMean = Sample<double>(offsetSample.sample, offsetSample.timestamp);
  }else{
    double i = packets;
    double x = offsetSample.sample;
    cumulativeMean.sample = ((i-1)*cumulativeMean.sample)/i + x/i;
    cumulativeMean.timestamp = offsetSample.timestamp;
  }
  return cumulativeMean.sample;
}

bool ServerOffsetCumulativeMean::isCalibrated(){
  if (successCount >= OFFSET_MEAN_CALIBRATION_CONSECUTIVE_PACKETS)
    return true;
  if (packets == 1)
    return false;

  double deltaX = cumulativeMean.sample - lastCumulativeMean.sample;
  double deltaT = cumulativeMean.timestamp - lastCumulativeMean.timestamp;
  bool success = abs(deltaX/deltaT) < OFFSET_MEAN_CALIBRATION_DERIVATIVE_THRESHOLD;
  if (success){
    successCount++;
  }else{
    successCount = 0;
  }
  return (successCount >= OFFSET_MEAN_CALIBRATION_CONSECUTIVE_PACKETS);
}

long ServerOffsetCumulativeMean::getMean(){
  return cumulativeMean.sample;
}

void ServerOffsetCumulativeMean::expireCalculation(){
  reset();
}

void ServerOffsetCumulativeMean::reset(){
  lastCumulativeMean = Sample<double>();
  cumulativeMean = Sample<double>();
  packets = 0;
  successCount = 0;
  conf = singleton(Configuration);
}

/******************************** TimeClock ***********************************/

#define EXPIRATION_PERIOD (configuration->ClockSync->expirationPeriod)
#define FIRST_PACKETS_IGNORE_QTY (configuration->ClockSync->firstPacketsIgnoreQty)

TimeClock::TimeClock(){
  setup();
}

void TimeClock::setup(){
  minimumOffset = 0xFFFFFFFF;
  expirationPeriodIndex = 0;
  offsetExpirationExponent = 0;
  correction = 0;
  firstInvalidPacketsQty = 0;
  serverOffsetCumulativeMean.reset();
  configuration = singleton(Configuration);
  synced = false;
}

bool TimeClock::isSynced(){
  return synced;
}

bool TimeClock::updateServerOffset(Frame* frame){

  /* ignore first packets due to errors in packet timestamping */
  if (firstInvalidPacketsQty < FIRST_PACKETS_IGNORE_QTY){
    firstInvalidPacketsQty++;
    return false;
  }

  /* update offset cumulative mean statistic */
  Sample<long> serverOffset = frame->getOffsetAgainstServerTime();
  double offsetMean = serverOffsetCumulativeMean.updateMean(serverOffset);

  if (!serverOffsetCumulativeMean.isCalibrated()){
    if (expirationPeriodIndex != 0){
      /* the clock was synced with server but it's calibrating due to expiration */
      DEBUG(Serial.printf("%i\t%lu\t%s\t%i\n", serverOffset, minimumOffset, String(offsetMean).c_str(),frame->seq));
      return true;
    } else {
      /* the clock hasn't ever been calibrated */
      return false;
    }
  }

  /*  if current offset sample is too far from the cumulative mean,
   *  consider it as an outlier.
   */
  if (abs(serverOffset.sample - offsetMean) > configuration->ClockSync->offsetSigma){
    DEBUG(
      if (expirationPeriodIndex != 0)
        Serial.printf("%i\t%lu\t%s\t%i\n", serverOffset, minimumOffset, String(offsetMean).c_str(),frame->seq);
    )
    return (expirationPeriodIndex != 0);
  }

  /*  if we arrived here, we have the cumulatveMean calibrated
   *  (i.e. the cumulativeMean has a representative value)
   *  and an offset sample that is not an outlier.
   */

  if (expirationPeriodIndex == 0) {
    Serial.printf("Calibrating...Finished!\n");
    /* multiplier was never calculated */
    correction = serverOffset.sample;
    expirationPeriodIndex = ((double)time()/(double)EXPIRATION_PERIOD) + 1;
    DEBUG(Serial.println("serverOffset\tminimumOffset\tmean\tseq"));
  }else{
    if (time() >= expirationPeriodIndex*EXPIRATION_PERIOD) {
      expirationPeriodIndex++;
      minimumOffset = ((unsigned long)(abs(correction) + pow(2,offsetExpirationExponent)));
      DEBUG(Serial.printf("Expiration -> minOff %i\n", minimumOffset));
      offsetExpirationExponent++;
      serverOffsetCumulativeMean.expireCalculation();
    }
  }

  if (((unsigned long)abs(serverOffset.sample)) <= minimumOffset){
    DEBUG(Serial.printf("Setting offset %i\n", serverOffset));
    minimumOffset = (unsigned long)abs(serverOffset.sample);
    correction = serverOffset.sample;
    offsetExpirationExponent = 0;
    synced = true;
  }

  DEBUG(Serial.printf("%i\t%lu\t%s\t%i\n", serverOffset.sample, minimumOffset, String(offsetMean).c_str(),frame->seq));
  return true;
}

unsigned long TimeClock::time(){
  bool isPositive = correction >= 0;
  unsigned long corr = (unsigned long) abs(correction);
  unsigned long milis = rawTime();
  return (unsigned long)(isPositive ? (milis + corr) : (milis - corr));
}

unsigned long TimeClock::rawTime(){
  return millis();
}

void TimeClock::configurationChanged(){
  Serial.println("TimeClock::configurationChanged()");
  setup();
}
