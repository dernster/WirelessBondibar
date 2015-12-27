#pragma once
#include "utils.h"
#include "Configuration.h"

class WifiManager : public ConfigurationObserver{
SINGLETON_H(WifiManager)
private:
  Configuration* configuration;
  void setup();
  bool connected;
public:
  WifiManager();
  void connect();
  void disconnect();
  void configurationChanged();
  ~WifiManager();
};
