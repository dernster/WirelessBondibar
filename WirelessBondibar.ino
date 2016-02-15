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
    configuration->Wifi->ssid = "G2_6031";
//    bondibar = singleton(Bondibar);
    ap = singleton(APServer);
    wifiManager = singleton(WifiManager);
//    controlServer = singleton(ControlServer);
//    streaming = singleton(Streaming);
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


byte packet[] = {0,0,0,0,0,0,255,255,255}; 
unsigned short seq = 0;
WiFiUDP udp;

time_r lastPacketTime = 0;
bool yaMovi = false;

inline void movePin(time_r time){
  if (((time % 1000) == 0) && !yaMovi){
    digitalWrite(NOTIFY_PIN,HIGH);
    digitalWrite(NOTIFY_PIN,LOW);
    yaMovi = true; /* to not move pin every time within 1ms */
    Serial.println("moviendo pata!");
  } else if ((time % 1000) != 0){
    yaMovi = false;
  }
}

void loop() {

  time_r time = modules->clock->time();
  movePin(time);

  if ((time - lastPacketTime) >= 42){
    
    lastPacketTime = time;
    seq++;
  
    if ((seq % 100) == 0) {
      packet[8] = 255 - packet[8];
      packet[7] = 255 - packet[7];
      packet[6] = 255 - packet[6];
    }
    time_r pt = time + 240;
    packet[0] = pt & 0xFF;
    packet[1] = (pt >> 8 ) & 0xFF;
    packet[2] = (pt >> 16) & 0xFF;
    packet[3] = (pt >> 24) & 0xFF;
    packet[4] = seq & 0xFF;
    packet[5] = (seq >> 8) & 0xFF;

    if (random(100) < 20){
      // simulate packet delay
      LOOP_UNTIL(10 + random(100)){
        time_r time = modules->clock->time();
        movePin(time);
      }
    }
    
  
      // send broadcast packet asking IP and Port of server
    IPAddress ip = WiFi.localIP();
    ip[3] = 255;
    int res = udp.beginPacket(ip,modules->configuration->Streaming->port);
    udp.write(packet,9);
    udp.endPacket();

  }
 
}


