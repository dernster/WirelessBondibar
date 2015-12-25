#include "WifiManager.h"

SINGLETON_CPP(WifiManager)
  
WifiManager::WifiManager(){
  configuration = singleton(Configuration);
  configuration->addObserver(this);
}

void WifiManager::connect(){
  disconnect();
  WiFi.begin(configuration->Wifi->ssid.c_str(),configuration->Wifi->password.c_str());
  while ( WiFi.status() != WL_CONNECTED) {
    Serial.println("Attempting to connect to SSID: " + configuration->Wifi->ssid);
    delay(500);
  }
  Serial.println("Connected to wifi");
  configuration->Wifi->ip = ipToString(WiFi.localIP());
  printWifiStatus();
}

void WifiManager::disconnect(){
  WiFi.disconnect();
}
  
void WifiManager::configurationChanged(){
  if (configuration->Wifi->ssid != String(WiFi.SSID())){
    Serial.println("WifiManager::configurationChanged()");
    connect();
  }
}

WifiManager::~WifiManager(){
  
}


  
