#pragma once
#include "utils.h"
#include "Configuration.h"
#include <WiFiUdp.h>
#include "TimeClock.h"

class Frame{
public:
  byte* data;
  int len;
  unsigned long pt;
  byte seq;
  bool expClkSync;

  ~Frame(){
    if (data)
      delete [] data;
  }
};

class Streaming : ConfigurationObserver{
SINGLETON_H(Streaming)
private:
  unsigned long start;
  unsigned long stop;
  unsigned long bytesReceived;
  unsigned long lastArrivedPacketTimestamp;
  Configuration* configuration;
  TimeClock* clock;
  void setup();
  vector<Frame*> buffer;
public:
  WiFiUDP udp;
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
