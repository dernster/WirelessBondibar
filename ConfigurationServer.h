#pragma once
#include "utils.h"
#include "Configuration.h"
#include <WiFiUdp.h>

class ConfigurationServer : public ConfigurationObserver{
SINGLETON_H(ConfigurationServer)
private:
  WiFiUDP udp;
  Configuration* configuration;
  char* buffer;
  void registerInServer();
  void setup();
  bool serverDiscovered;
public:
  ConfigurationServer();
  bool incomingCommand();
  void processCommand();
  void obtainServerEndpoint();
  void configurationChanged();
  void commandReceivedFlashLed();
  ~ConfigurationServer();
};
