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

#define NOTIFY_PIN 10

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
    configuration->Wifi->password = "sudosudo";
    bondibar = singleton(Bondibar);
    ap = singleton(APServer);
    wifiManager = singleton(WifiManager);
    controlServer = singleton(ControlServer);
    bondibar->setup();
    streaming = singleton(Streaming);
    clock = singleton(TimeClock);
  };

  void reset() {
    firstTime = true;
    streaming->udp.stop();
    clock->setup();
    bondibar->turnOffLights();
    configuration->notifyObservers();
  };

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
  bool setPowerSavingOff = wifi_set_sleep_type(NONE_SLEEP_T);

  Serial.begin(9600);
  while (!Serial) {
    yield();
  }

  Serial.printf("Set wifi power save OFF -> ");
  setPowerSavingOff ? Serial.println("SUCCESS!") : Serial.println("ERROR");

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

  WiFi.mode(WIFI_STA);

  Serial.setDebugOutput(true);

  delay(500);
  modules = new Modules();

}
//-------------------------------------------------

unsigned long packetsPlayedQty = 0, lastPacketPlaybackRawTime = 0, packetPlaybackDiffSum = 0
            , packetPlaybackDiffSumPow = 0;
byte previousPacketSeq = 0;
bool previousWasGenerated = false;
bool value = true;
void loop() {

  // move pin in specific times
  // unsigned long time = modules->clock->time();
  // if (modules->clock->isSynced() && msIsMultiple(time,1000)){
  //   digitalWrite(NOTIFY_PIN,value = !value);
  //   // Serial.println("moviendo pata!");
  // }

  // if (msIsMultiple(time, 5*60*1000)){
  //   system_print_meminfo();
  // }


  if (modules->streaming->frame()){

    modules->streaming->bufferFrame();

  } else if((playFrame = modules->streaming->frameToPlay()) != NULL){

      // move pin in specific times
      // unsigned long time = modules->clock->time();
      if (modules->clock->isSynced() && (playFrame->seq % 32) == 0){
        digitalWrite(NOTIFY_PIN,value = !value);
        // Serial.println("moviendo pata!");
      }

      unsigned long currentFrameRawPt = modules->clock->rawTime();
      modules->bondibar->sendData(playFrame->data, playFrame->len);

      // if (playFrame->isGeneratedFrame)
      //   Serial.println("PLAYED A GENERATED FRAME!");

      if (packetsPlayedQty > 0 && previousPacketSeq + 1 == playFrame->seq && !previousWasGenerated && !playFrame->isGeneratedFrame) {

        unsigned long currentDifference = currentFrameRawPt - lastPacketPlaybackRawTime;

        if (currentDifference > modules->configuration->Stats->ptFrameRateMax) {
          // Serial.printf("MAX: %lu %i %i\n", currentDifference, previousPacketSeq, playFrame->seq);
          modules->configuration->Stats->ptFrameRateMax = currentDifference;
        }

        if (currentDifference < modules->configuration->Stats->ptFrameRateMin) {
          // Serial.printf("MIN: %lu %i %i\n", currentDifference, previousPacketSeq, playFrame->seq);
          modules->configuration->Stats->ptFrameRateMin = currentDifference;
        }

        packetPlaybackDiffSum += currentDifference;
        packetPlaybackDiffSumPow += ((unsigned long) pow(currentDifference, 2));
        modules->configuration->Stats->ptFrameRateMean = (double)packetPlaybackDiffSum / (double)(packetsPlayedQty + 1);
        modules->configuration->Stats->ptFrameRateStdev = sqrt(
          ((double)packetPlaybackDiffSumPow / (double)packetsPlayedQty) - pow((double)packetPlaybackDiffSum / (double)packetsPlayedQty, 2));

        packetsPlayedQty++;
      } else if (packetsPlayedQty == 0) {
        packetsPlayedQty++;
      }

      previousWasGenerated = playFrame->isGeneratedFrame;
      previousPacketSeq = playFrame->seq;
      lastPacketPlaybackRawTime = currentFrameRawPt;
      delete playFrame;

  }else if (modules->controlServer->incomingCommand()){

    modules->controlServer->processCommand();

  }else if (!modules->streaming->active){

    modules->ap->handleClient();

    if (!modules->controlServer->serverIsAlive){
      /* server is dead */
      flashLed(300,250,3);
      Serial.println("SERVER IS DEAD! Reseting modules!");
      ESP.restart();
      // modules->reset();
    }
  }
}
