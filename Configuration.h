#pragma once
#include "utils.h"

class Configuration{
SINGLETON_H(Configuration)
public:
  String ssid;
  String password;
  
  int deviceNumber;
  int pixelsQty;
  int streamingPort;
  int commandsPort;
  int commandPacketLength;
};



