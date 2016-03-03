#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
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
  #include "ets_sys.h"
  #include "os_type.h"
  #include "osapi.h"
  #include "mem.h"
  #include "user_interface.h"
  #include "cont.h"
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
    clock->reset();
    bondibar->turnOffLights();
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

  pinMode(NOTIFY_PIN,OUTPUT);
  digitalWrite(NOTIFY_PIN, LOW);

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

  // move pin in specific times
  time_r time = modules->clock->time();
  if (msIsMultiple(time,1000)){
    digitalWrite(NOTIFY_PIN,HIGH);
    digitalWrite(NOTIFY_PIN,LOW);
    // Serial.println("moviendo pata!");
  }

  // if (msIsMultiple(time, 5*60*1000)){
  //   system_print_meminfo();
  // }

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
      flashLed(300,250,3);
      Serial.println("SERVER IS DEAD! Reseting modules!");
      modules->reset();
    }
  }
}
