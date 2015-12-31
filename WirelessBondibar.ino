#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include "Bondibar.h"
#include "Configuration.h"
#include "Streaming.h"
#include "ConfigurationServer.h"
#include "utils.h"
#include "WifiManager.h"
#include "APServer.h"
#include "Storage.h"

#define DEBUG

struct Modules{

  Modules(){

    configuration = singleton(Configuration);
    singleton(Storage)->readSSIDAndPassword(configuration->Wifi->ssid,configuration->Wifi->password);
    configuration->Device->number = singleton(Storage)->readDeviceID();
    configuration->Device->managedPixelsQty = 8;
    configuration->Device->firstPixel = configuration->Device->number*configuration->Device->managedPixelsQty;
    configuration->Streaming->port = 7788;
    configuration->ConfigurationServer->discoveryPort = 9999;
    
    ap = singleton(APServer);
    wifiManager = singleton(WifiManager);
    streaming = singleton(Streaming);
    configurationServer = singleton(ConfigurationServer);
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

void setup() {

  // configure LED
  pinMode(LED,OUTPUT);
  digitalWrite(LED,HIGH);
  flashLed(300,250,3);
  
  Serial.begin(9600);
  while (!Serial) {}
  
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);  
  delay(500);
    
  Serial.setDebugOutput(true);
  modules = new Modules();
}

void loop() {

  if (modules->streaming->frame()){ /* is there an incoming frame? */

    digitalWrite(LED,LOW);
    modules->streaming->readFrame(); 
    modules->bondibar->sendData(modules->streaming->buffer,modules->configuration->Device->firstPixel,modules->configuration->Device->managedPixelsQty*3); 
    digitalWrite(LED,HIGH);
    
  }else if (!modules->streaming->active){ /* if streaming is not active == 5 seconds of inactivity */

    /* take care of lower priority operations */
    if(modules->configurationServer->incomingCommand()){ /* is there an incoming configuration command? */
      modules->configurationServer->processCommand();
    }else{ /* is there someone connecting to the AP? */
      modules->ap->handleClient();
    }
  }
}


