#pragma once
#include "utils.h"
#include "Configuration.h"
#include <WiFiUdp.h>
#include "TimeClock.h"
#include "DataTypes.h"

#define FRAMES_TO_INTERPOLATE (1)

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
  vector<Frame*> lastFramesBuffer;
  int lastFrameIndex;
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
