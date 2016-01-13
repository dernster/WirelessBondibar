#pragma once
#include "utils.h"
#include "Configuration.h"
#include <WiFiUdp.h>
#include "TimeClock.h"

class Frame{
public:
  byte* data;
  int len;
  time_r pt;
  short int seq;

  ~Frame(){
    if (data)
      delete [] data;
  }
};

class Streaming : ConfigurationObserver{
SINGLETON_H(Streaming)
private:
  WiFiUDP udp;
  unsigned long start;
  unsigned long stop;
  unsigned long bytesReceived;
  Configuration* configuration;
  TimeClock* clock;
  void setup();
  vector<Frame*> buffer;
public:
  Streaming();
  void configure();
  bool frame();
  void bufferFrame();
  Frame* frameToPlay();
  byte* dataBuffer;
  int packetSize;
  void configurationChanged();
  void updateBufferStat();
  ~Streaming();
  bool active;
};


