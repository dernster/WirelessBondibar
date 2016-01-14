#pragma once
#include "utils.h"
#include "Configuration.h"
#include <WiFiUdp.h>
//#include <WiFi.h>

#pragma pack(push)
#pragma pack(1)
struct SenderoControlHeader{
  char packetName[8];
  bool requestClockFlag :1;
  bool clockCorrectionOffsetFlag :1;
  bool configurationFlag :1;
  int  padding :5;

  int size(){
    return sizeof(SenderoControlHeader);
  }

  String toString(){
    String name = String(packetName) + "\n";
    String v1 = String("isRequestClock = ") + (requestClockFlag ? "Yes" : "No") + "\n"; 
    String v2 = String("clockCorrectionOffsetFlag = ") + (clockCorrectionOffsetFlag ? "Yes" : "No") + "\n"; 
    String v3 = String("configurationFlag = ") + (configurationFlag ? "Yes" : "No") + "\n"; 
    return name + v1 + v2 + v3;
  }
};
#pragma pack(pop)

class ControlServer : public ConfigurationObserver{
SINGLETON_H(ControlServer)
private:
  WiFiUDP udp;
  WiFiServer* server;
  WiFiClient client;
  Configuration* configuration;
  char* buffer;
  void registerInServer();
  void setup();
  bool serverDiscovered;
public:
  ControlServer();
  bool incomingCommand();
  void processCommand();
  void obtainServerEndpoint();
  void configurationChanged();
  void commandReceivedFlashLed();
  ~ControlServer();
};
