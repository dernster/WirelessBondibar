#pragma once
#include "utils.h"
#include "Configuration.h"
#include <WiFiUdp.h>


#pragma pack(push)
#pragma pack(1)
struct SenderoControlHeader{
  
  char packetName[7];
  union{
    struct{
      bool requestClockFlag :1;
      bool clockCorrectionOffsetFlag :1;
      bool configurationFlag :1;
      int  padding :4;
      bool isResponse :1;
    };
    byte falgs;
  };
  

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
    return headerName + v1 + v2 + v3;
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

  template<typename T> T readBuffer(void* buffer);
  template<typename T> void writeBuffer(void* buffer, T data);
public:
  ControlServer();
  bool incomingCommand();
  void processCommand();
  void obtainServerEndpoint();
  void configurationChanged();
  void commandReceivedFlashLed();
  ~ControlServer();
};
