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

bool firstTime = true;

struct Modules{

  Modules(){

    /* create modules */
    configuration = singleton(Configuration);
    configuration->Wifi->ssid = "/dev/null";
    configuration->Wifi->password = "$sudo\\s!!";
    bondibar = singleton(Bondibar);
    ap = singleton(APServer);
    wifiManager = singleton(WifiManager);
    controlServer = singleton(ControlServer);
    streaming = singleton(Streaming);
    clock = singleton(TimeClock);
  }

  void reset(){
    firstTime = true;
    streaming->udp.stop();
    clock->setup();
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

  /* set CPU frequency */
  bool setFrequencyOK = system_update_cpu_freq(160);

  Serial.begin(9600);
  while (!Serial) {}

  Serial.printf("Set frequency at 160MHz -> ");
  setFrequencyOK ? Serial.println("SUCCESS!") : Serial.println("ERROR");

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

  // WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  // delay(500);

  Serial.setDebugOutput(true);

  delay(500);
  modules = new Modules();
}
//-------------------------------------------------

bool value = true;
void loop() {

  if (firstTime){
    firstTime = false;
    modules->streaming->udp.begin(modules->configuration->Streaming->port);
    int count = 0;
    while(true){
      if (modules->streaming->udp.parsePacket()){
        modules->streaming->udp.flush();
        count++;
      }
      if (count == 24*7)
        break;
    }
  }

  // move pin in specific times
  unsigned long time = modules->clock->time();
  if (msIsMultiple(time,1000)){
    digitalWrite(NOTIFY_PIN,value = !value);
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
