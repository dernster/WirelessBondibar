#pragma once
#include "utils.h"
#include "HardwareSerial.h"

class DebugClass : public HardwareSerial{
private:
  bool enabled;
public:
  DebugClass();
  void init();
  void printMemoryInfoEvery(time_t ms);
};

extern DebugClass Debug;
