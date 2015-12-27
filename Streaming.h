#pragma once
#include "utils.h"
#include "Configuration.h"
#include <WiFiUdp.h>

class Streaming : ConfigurationObserver{
SINGLETON_H(Streaming)
private:
  WiFiUDP udp;
  Configuration* configuration;
  void setup();
public:
  Streaming();
  void configure();
  bool frame();
  void readFrame();
  byte* buffer;
  int packetSize;
  void configurationChanged();
  ~Streaming();
  bool active;
};


