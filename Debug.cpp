#include "Debug.h"
#include "Arduino.h"
#include "Configuration.h"
#include "TimeClock.h"

extern "C" {
  #include "ets_sys.h"
  #include "os_type.h"
  #include "osapi.h"
  #include "mem.h"
  #include "user_interface.h"
  #include "cont.h"
}

DebugClass::DebugClass() : HardwareSerial(0){
  enabled = true;
};

void DebugClass::init(){
  enabled = true;
  begin(9600);
  while (!(*this)) {}
  setDebugOutput(true);
  pinMode(LED,OUTPUT);
  digitalWrite(LED,HIGH);
  flashLed(300,250,3);
}

void DebugClass::printMemoryInfoEvery(time_t ms){
  if (!enabled)
    return;

  if (msIsMultiple(singleton(TimeClock)->rawTime(), ms)){
    system_print_meminfo();
  }
}

DebugClass Debug;
