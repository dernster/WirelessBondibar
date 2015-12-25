#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "Bondibar.h"
#include "Configuration.h"
#include "Streaming.h"
#include "ConfigurationServer.h"
#include "utils.h"
#include "WifiManager.h"

byte bufferToBondi[24];

void initConfig(){
  Configuration* configuration = singleton(Configuration);
  
  configuration->Wifi->ssid = "/dev/null";
  configuration->Wifi->password = "$sudo\\s!!";
  configuration->Device->number = 0;
  configuration->Global->pixelsQty = 200; /* not set yet */
  configuration->Streaming->port = 7788;
  configuration->ConfigurationServer->discoveryPort = 9999;
  configuration->ConfigurationServer->packetLength = 200;
}

void initStreaming(){
  singleton(Streaming);
}

void initConfigServer(){
  singleton(ConfigurationServer);
}

void initWifiManager(){
  singleton(WifiManager)->connect();
}


void setup() {
  Serial.begin(9600);
  while (!Serial) {}
  
  // configure LED
  pinMode(LED,OUTPUT);
  digitalWrite(LED,HIGH);
  flashLed(300,250,3);
  
  initConfig();
  initWifiManager();
  initStreaming();
  initConfigServer();
}

void loop() {
  
  digitalWrite(LED,HIGH);
  
  if (singleton(Streaming)->frame()){
    
    digitalWrite(LED,LOW);
    singleton(Streaming)->readFrame(); 
    int start = singleton(Configuration)->Device->number*100*3;
    Bondibar.sendData(singleton(Streaming)->buffer,start,24); 
    
  }else if (singleton(ConfigurationServer)->incomingCommand()){
    
    singleton(ConfigurationServer)->processCommand();
  
  }
}


