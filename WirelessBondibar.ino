#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include "Storage.h"
#include "Bondibar.h"
#include "Configuration.h"
#include "Streaming.h"
#include "ConfigurationServer.h"
#include "utils.h"
#include "WifiManager.h"
#include "APServer.h"

extern "C" {
#include "user_interface.h"
}

#define DEBUG

struct Modules{

  Modules(){

    configuration = singleton(Configuration);;
    configuration->Device->managedPixelsQty = 8;
    configuration->Device->firstPixel = configuration->Device->number*configuration->Device->managedPixelsQty;
    configuration->Streaming->port = 7788;
    configuration->ConfigurationServer->discoveryPort = 9999;

    /* read configs from EEPROM */
    singleton(Storage)->readSSIDAndPassword(configuration->Wifi->ssid,configuration->Wifi->password);
    configuration->Device->number = singleton(Storage)->readDeviceID();

    /* create modules */
    ap = singleton(APServer);
    wifiManager = singleton(WifiManager);
    // streaming = singleton(Streaming);
    // configurationServer = singleton(ConfigurationServer);
    bondibar = singleton(Bondibar); 
  }
  
  Configuration* configuration;
  APServer* ap;
  WifiManager* wifiManager;
  Streaming* streaming;
  ConfigurationServer* configurationServer;
  Bondibar* bondibar; 
};

Modules* modules;

#define time_r unsigned long

class TimeClock{
public:
  long correction;

  void addCorrection(long offset){
    correction += offset;
    alarmStart += offset;
  }

  TimeClock(){
    correction = 0;
  }

  time_r time(){
    return ((time_r)millis()) + correction;
  }

  time_r alarmStart;
  time_r alarmInterval;
  void setAlarmIn(time_r ms){
    alarmStart = time();
    alarmInterval = ms;
  }

  bool alarm(){
    time_r currentTime = time();
    if (currentTime >= alarmStart)
      return ((currentTime - alarmStart) >= alarmInterval);
    else
      return ((0xFFFFFFFF - alarmStart) + currentTime >= alarmInterval);
  }
};

TimeClock clock;
WiFiUDP udpRecv;
WiFiUDP udpSend;
char buffer[200];
byte white[24] = {0};
byte black[24] = {0};



void setup() {

  for(int i = 0; i < 24; i++){
    white[i] = 255;
    black[i] = 0;
  }
  // configure LED
  pinMode(LED,OUTPUT);
  digitalWrite(LED,HIGH);
  flashLed(300,250,3);

  /* set hostname, this works only from setup function */
  String str = "WBB-" + String(singleton(Storage)->readDeviceID());
  char * hostName = new char[str.length() + 1];
  strcpy(hostName, str.c_str());
  hostName[str.length()] = '\0';
  wifi_station_set_hostname(hostName);
  delete[] hostName;
  
  Serial.begin(9600);
  while (!Serial) {}
  
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);  
  delay(500);
    
  Serial.setDebugOutput(true);
  modules = new Modules();


  udpRecv.begin(8888);

  clock.setAlarmIn(10000);
}
//-------------------------------------------------







void loop() {

  int size = udpRecv.parsePacket();
  if (size){

    Serial.println("hay paquete!");

    udpRecv.read(buffer,size);
    buffer[size] = '\0';

    String command(buffer);

    Serial.println(command);

    if (command == "clock_request"){

      /* master is requesting my time */
      long t = clock.time();
      Serial.println("time requested! " + String(t));

      IPAddress ip(192,168,1,100);
      int res = udpSend.beginPacket(ip,8889);
      String data(t);
      udpSend.write(data.c_str(),data.length());
      udpSend.endPacket();

    }else if (command.startsWith("adjust_clock")){

      vector<String> split = splitString(command,' ');
      long offset = split[1].toInt();
      Serial.println(String("time ") + String(clock.time()));
      clock.addCorrection(offset);
      Serial.println(String("time adjsted ") + String(clock.time()));
    }
  }
  if (clock.alarm()){
    Serial.println("Alram!");
    modules->bondibar->sendData(white,24);
    clock.setAlarmIn(1000);
    while (!clock.alarm());
    modules->bondibar->sendData(black,24);
    clock.setAlarmIn(10000);
    
  }













  // if (modules->streaming->frame()){ /* is there an incoming frame? */

  //   digitalWrite(LED,LOW);
  //   modules->streaming->readFrame();
  //   //modules->bondibar->sendData(modules->streaming->buffer,modules->configuration->Device->firstPixel,modules->configuration->Device->managedPixelsQty*3); 
  //   frame f;
  //   f.len = modules->configuration->Device->managedPixelsQty*3;
  //   f.data = new byte[f.len];
  //   for(int i = 0; i < f.len; i++){
  //     f.data[i] = modules->streaming->buffer[modules->configuration->Device->firstPixel + i];
  //   }
  //   f.seq = modules->streaming->buffer[0] + (modules->streaming->buffer[1] << 8);
  //   f.scheduleTime = f.seq + delta;



  //   digitalWrite(LED,HIGH);



    
  // }else if (!modules->streaming->active){ /* if streaming is not active == 5 seconds of inactivity */

  //   /* take care of lower priority operations */
  //   if(modules->configurationServer->incomingCommand()){ /* is there an incoming configuration command? */
  //     modules->configurationServer->processCommand();
  //   }else{ /* is there someone connecting to the AP? */
  //     modules->ap->handleClient();
  //   }
  // }
}


