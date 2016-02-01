#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include "Storage.h"
#include "Bondibar.h"
#include "Configuration.h"
#include "Streaming.h"
#include "ControlServer.h"
#include "utils.h"
#include "WifiManager.h"
#include "APServer.h"
#include "TimeClock.h"

extern "C" {
#include "user_interface.h"
}

//#define DEBUG

struct Modules{

  Modules(){

    /* create modules */
    configuration = singleton(Configuration);
    ap = singleton(APServer);
    wifiManager = singleton(WifiManager);
    controlServer = singleton(ControlServer);
    streaming = singleton(Streaming);
    bondibar = singleton(Bondibar); 
    clock = singleton(TimeClock);
  }

  void reset(){
    streaming->udp.stop();
    configuration->notifyObservers();
  }
  
  Configuration* configuration;
  APServer* ap;
  WifiManager* wifiManager;
  Streaming* streaming;
  ControlServer* controlServer;
  Bondibar* bondibar;
  TimeClock* clock; 
};

Modules* modules;
Frame* playFrame;

void setup() {
  
  Serial.begin(9600);
  while (!Serial) {}
  
  // configure LED
  pinMode(LED,OUTPUT);
  digitalWrite(LED,HIGH);
  flashLed(300,250,3);

  /* set hostname, this works only from setup function */
  String str = "WBB-" + String(singleton(Configuration)->Device->number);
  char * hostName = new char[str.length() + 1];
  strcpy(hostName, str.c_str());
  hostName[str.length()] = '\0';
  wifi_station_set_hostname(hostName);
  delete[] hostName;
  
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);  
  delay(500);
    
  Serial.setDebugOutput(true);
  modules = new Modules();
}
//-------------------------------------------------




void loop() {

  if (modules->streaming->frame()){

    modules->streaming->bufferFrame();

  }else if((playFrame = modules->streaming->frameToPlay()) != NULL){

    modules->bondibar->sendData(playFrame->data,playFrame->len);
    delete playFrame;

  }else if (modules->controlServer->incomingCommand()){
    
    modules->controlServer->processCommand();
    
  }else if (!modules->streaming->active){

    modules->ap->handleClient();
    
    if (!modules->controlServer->serverIsAlive){
      /* server is dead */
      modules->reset();
    }
  }
}


