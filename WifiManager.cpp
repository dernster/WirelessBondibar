#include "WifiManager.h"
#include "APServer.h"

SINGLETON_CPP(WifiManager)
  
WifiManager::WifiManager(){
  configuration = singleton(Configuration);
  configuration->addObserver(this);
  setup();
}

void WifiManager::setup(){
  connected = false;
  this->connect();
}

void WifiManager::connect(){
  disconnect();
  APServer* ap = singleton(APServer);
  WiFi.begin(configuration->Wifi->ssid.c_str(),configuration->Wifi->password.c_str());
  Serial.println("Attempting to connect to SSID " + configuration->Wifi->ssid + "...");
  while ( WiFi.status() != WL_CONNECTED) {
    LOOP_UNTIL(2000){
      ap->handleClient(); /* allow connections to AP, in case someone needs to change ssid */
    }
  }
  if (!connected){ /* to not print twice in case of connection from the AP */
    connected = true;
    Serial.println("Connected to wifi");
    configuration->Wifi->ip = ipToString(WiFi.localIP());
    printWifiStatus();
  }
}

void WifiManager::disconnect(){
  WiFi.disconnect();
  connected = false;
}
  
void WifiManager::configurationChanged(){
  if (configuration->Wifi->ssid != String(WiFi.SSID())){
    Serial.println("WifiManager::configurationChanged()");
    setup();
  }
}

WifiManager::~WifiManager(){
  
}


  
