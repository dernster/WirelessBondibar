#pragma once
#include "utils.h"
#include "Configuration.h"

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

class APServer : public ConfigurationObserver {
SINGLETON_H(APServer)
private:
  ESP8266WebServer* server;
  static String buildPage();
  static void handleRoot();
  static void handleSave();
  static String apIP;
  void setup();
public:
  void configurationChanged();
  void handleClient();
  APServer();
};

