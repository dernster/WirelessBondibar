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

#define NOTIFY_PIN 5

extern "C" {
#include "user_interface.h"
}


struct Modules{

  Modules(){

    /* create modules */
    configuration = singleton(Configuration);
    bondibar = singleton(Bondibar);
    ap = singleton(APServer);
    wifiManager = singleton(WifiManager);
    controlServer = singleton(ControlServer);
    streaming = singleton(Streaming);
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
  for(int i = 0; i < NEH; i++)
    buffer[i] = 34;
    
  Serial.begin(9600);
  while (!Serial) {}

  pinMode(NOTIFY_PIN,OUTPUT);
  digitalWrite(NOTIFY_PIN,LOW);
  
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
    
//  Serial.setDebugOutput(true);
  modules = new Modules();
}
//-------------------------------------------------



bool yaMovi = false;

void loop() {

  // move pin in specific times 
  time_r time = modules->clock->time();
  if (((time % 1000) == 0) && !yaMovi){
    digitalWrite(NOTIFY_PIN,HIGH);
    digitalWrite(NOTIFY_PIN,LOW);
    yaMovi = true; /* to not move pin every time within 1ms */
//    Serial.println("moviendo pata!");
  } else if ((time % 1000) != 0){
    yaMovi = false;
  }

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


