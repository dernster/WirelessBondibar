#pragma once
#include "utils.h"
#include "Configuration.h"
#include <WiFiUdp.h>

class ConfigurationServer{
SINGLETON_H(ConfigurationServer)
private:
  WiFiUDP udp;
  Configuration* configuration;
  char* buffer;
  void becomeVisibleToNetwork();
public:
  ConfigurationServer();
  bool incomingCommand();
  void processCommand();
};
