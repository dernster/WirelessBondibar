#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "Bondibar.h"
#include "Configuration.h"
#include "Streaming.h"
#include "ConfigurationServer.h"
#include "utils.h"

Configuration* configuration;
ConfigurationServer* configServer;
Streaming* streaming;

#define LED 2

byte bufferToBondi[24];

void initConfig(){
  configuration = singleton(Configuration);
  configuration->ssid = "/dev/null";
  configuration->password = "$sudo\\s!!";
  configuration->deviceNumber = 0;
  configuration->pixelsQty = 200; /* not set yet */
  configuration->streamingPort = 7788;
  configuration->commandsPort = 9999;
  configuration->commandPacketLength = 20;
}

void initStreaming(){
  streaming = singleton(Streaming);
}

void initConfigServer(){
  configServer = singleton(ConfigurationServer);
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {}

  initConfig();
  connectToWifi();
  initStreaming();
  initConfigServer();

  // configure LED
  pinMode(LED,OUTPUT);
  digitalWrite(LED,HIGH);
}

void loop() {
  
  digitalWrite(LED,HIGH);
  
  if (streaming->frame()){
    
    digitalWrite(LED,LOW);
    streaming->readFrame(); 
    int start = configuration->deviceNumber*100*3;
    Bondibar.sendData(streaming->buffer,start,24); 
    
  }else if (configServer->incomingCommand()){
    
    configServer->processCommand();
  
  }
}


