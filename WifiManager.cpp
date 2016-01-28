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

  if (!connected){ /* to not print twice in case of connection from the AP */
    
    disconnect();
    APServer* ap = singleton(APServer);
    WiFi.begin(configuration->Wifi->ssid.c_str(),configuration->Wifi->password.c_str());
    Serial.println("Attempting to connect to SSID " + configuration->Wifi->ssid + "... password=" + configuration->Wifi->password + " " + String(configuration->Wifi->password.length()));
    while ( WiFi.status() != WL_CONNECTED) {
      LOOP_UNTIL(2000){
        ap->handleClient(); /* allow connections to AP, in case someone needs to change ssid */
      }
    }
    connected = true;
    Serial.println("Connected to wifi");
    configuration->Wifi->ip = ipToString(WiFi.localIP());
    printWifiStatus();

    ssid = configuration->Wifi->ssid;
    password = configuration->Wifi->password;
  }
}

void WifiManager::disconnect(){
  WiFi.disconnect();
  connected = false;
}
  
void WifiManager::configurationChanged(){
  String lastSSID = ssid;
  String lastPassword = password;
  if ((configuration->Wifi->ssid != lastSSID) || (configuration->Wifi->password != lastPassword)){
    singleton(Storage)->setSSIDAndPassword(configuration->Wifi->ssid,configuration->Wifi->password);
    Serial.println("WifiManager::configurationChanged()");
    setup();
  }
}

WifiManager::~WifiManager(){
  
}


  
