#pragma once
#include "utils.h"

class SMA{
private:
  long* window;
  int windowSize;
  int currentIndex;
  long sma;

  /* first window */
  long firstWindowCumulativeAverage;
  int firstWindowSampleCount;


public:
  inline bool isFirstWindow(){
    return (firstWindowSampleCount < windowSize);
  }

  SMA(int windowSize){
    this->windowSize = windowSize;
    window = new long[windowSize];
    currentIndex = 0;
    sma = 0;
    firstWindowSampleCount = 0;
    firstWindowCumulativeAverage = 0;
  }

  long addSample(long sample){
    long oldSample = window[currentIndex];
    window[currentIndex] = sample;
    currentIndex = (currentIndex + 1) % windowSize;

    if (isFirstWindow()){
      firstWindowSampleCount++;
      if (firstWindowSampleCount == 1){
        firstWindowCumulativeAverage = sample;
      }else{
        double i = firstWindowSampleCount;
        firstWindowCumulativeAverage = round(((i-1)*firstWindowCumulativeAverage)/i + (double)sample/i);
      }
      sma = firstWindowCumulativeAverage;
    }else{ /* we have one full window at least */
      sma = sma + round((double)sample/(double)windowSize - (double)oldSample/(double)windowSize);
    }
    return sma;
  }

  long currentSMA(){
    return sma;
  }
};

class TimeClock{
SINGLETON_H(TimeClock)
private:
  SMA* serverOffsetsSMACalculator;
  SMA* getServerOffsetsSMACalculator(){
    if (serverOffsetsSMACalculator == NULL)
      serverOffsetsSMACalculator = new SMA(24*5);
    return serverOffsetsSMACalculator;
  }
public:
  long correction;

  void addServerOffsetSample(time_t serverOffset){
    static int n = 0;
    n++;
    correction = getServerOffsetsSMACalculator()->addSample(serverOffset);
    // if ((n%(24*2)) == 0)
    //   Serial.println(correction);
  }

  TimeClock(){
    correction = 0;
    serverOffsetsSMACalculator = NULL;
  }

  time_r time(){
    return ((time_r)millis()) + correction;
  }

  time_r rawTime(){
    return millis();
  }

};
