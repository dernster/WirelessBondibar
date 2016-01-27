#pragma once
#include "utils.h"
#include "Configuration.h"
#include <WiFiUdp.h>


#pragma pack(push)
#pragma pack(1)
struct SenderoControlHeader{

  enum Command {
    
    REQUEST_CLOCK,
    CLOCK_CORRECTION,
    CONFIGURATION,
    NO_COMMAND,
  };
  
  char packetName[7];
  union{
    struct{
      bool requestClockFlag :1;
      bool clockCorrectionOffsetFlag :1;
      bool configurationFlag :1;
      bool closeConnectionFlag :1;
      bool requestStatsFlag :1;
      bool keepAliveFlag :1;
      int  padding :2;
    };
    byte flags;
  };

  Command type(){
    if (requestClockFlag)
      return REQUEST_CLOCK;
    if (clockCorrectionOffsetFlag)
      return CLOCK_CORRECTION;
    if (configurationFlag)
      return CONFIGURATION;
    return NO_COMMAND;
  }

  int size(){
    return sizeof(SenderoControlHeader);
  }

  String toString(){
    char name[8]; name[7] = '\0';
    for(int i = 0; i < 7; i++) name[i] = packetName[i]; 
    
    String headerName = String(name) + "\n";
    String v1 = String("isRequestClock = ") + (requestClockFlag ? "Yes" : "No") + "\n"; 
    String v2 = String("clockCorrectionOffsetFlag = ") + (clockCorrectionOffsetFlag ? "Yes" : "No") + "\n"; 
    String v3 = String("configurationFlag = ") + (configurationFlag ? "Yes" : "No") + "\n"; 
    String v4 = String("closeConnectionFlag = ") + (closeConnectionFlag ? "Yes" : "No") + "\n";
    String v5 = String("requestStatsFlag = ") + (requestStatsFlag ? "Yes" : "No") + "\n";
    return headerName + v1 + v2 + v3 + v4 + v5;
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
  unsigned long lastPacketTime;

  template<typename T> T readBuffer(void* buffer);
  template<typename T> void writeBuffer(void* buffer, T data);
public:
  bool serverIsAlive;
  void externalCommandReceived();
  ControlServer();
  bool incomingCommand();
  SenderoControlHeader processCommand();
  void obtainServerEndpoint();
  void configurationChanged();
  void commandReceivedFlashLed();
  ~ControlServer();
};
