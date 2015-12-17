#pragma once
#include "utils.h"
#include "Configuration.h"
#include <WiFiUdp.h>

class Streaming{
SINGLETON_H(Streaming)
private:
  WiFiUDP udp;
  Configuration* configuration;
public:
  Streaming();
  void configure();
  bool frame();
  void readFrame();
  byte* buffer;
  int packetSize;
};


