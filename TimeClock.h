#pragma once
#include "utils.h"

class TimeClock{
SINGLETON_H(TimeClock)
public:
  long correction;

  void addCorrection(long offset){
    correction += offset;
    alarmStart += offset;
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

  time_r alarmStart;
  time_r alarmInterval;
  void setAlarmIn(time_r ms){
    alarmStart = time();
    alarmInterval = ms;
  }

  bool alarm(){
    time_r currentTime = time();
    if (currentTime >= alarmStart)
      return ((currentTime - alarmStart) >= alarmInterval);
    else
      return ((0xFFFFFFFF - alarmStart) + currentTime >= alarmInterval);
  }
};
