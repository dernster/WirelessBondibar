#include <Arduino.h>
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


  void reset();

void setup();

inline void movePin(time_r time);

void loop();
#line 18 "WirelessBondibar.ino"
extern "C" {
#include "user_interface.h"
}


struct Modules{

Modules(){


configuration = singleton(Configuration);
configuration->Wifi->ssid = "/dev/null";
configuration->Wifi->password = "sudosudo";

ap = singleton(APServer);
wifiManager = singleton(WifiManager);


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

#define HEADER_SIZE (6)
#define PACKET_SIZE (HEADER_SIZE + 90*3)

byte whitePacket[PACKET_SIZE] = {0};
byte blackPacket[PACKET_SIZE] = {0};

void setup() {

Serial.begin(9600);
while (!Serial) {}

pinMode(NOTIFY_PIN,OUTPUT);
digitalWrite(NOTIFY_PIN,LOW);


pinMode(LED,OUTPUT);
digitalWrite(LED,HIGH);
flashLed(300,250,3);


String str = "WBB-" + String(singleton(Configuration)->Device->number);
char * hostName = new char[str.length() + 1];
strcpy(hostName, str.c_str());
hostName[str.length()] = '\0';
wifi_station_set_hostname(hostName);
delete[] hostName;

WiFi.disconnect();
WiFi.mode(WIFI_OFF);
delay(500);

for(int i = HEADER_SIZE; i < PACKET_SIZE; i++){
blackPacket[i] = 0;
whitePacket[i] = 255;
}


modules = new Modules();
}


byte seq = 0;
WiFiUDP udp;

time_r lastPacketTime = 0;
bool yaMovi = false;
bool pinState = true;

inline void movePin(time_r time){
if (((time % 1000) == 0) && !yaMovi){
digitalWrite(NOTIFY_PIN,pinState = !pinState);
yaMovi = true;
Serial.println("moviendo pata!");
} else if ((time % 1000) != 0){
yaMovi = false;
}
}

byte* currentPacket = whitePacket;

void loop() {

unsigned long time = millis();
movePin(time);

if ((time - lastPacketTime) >= 42){

lastPacketTime = time;
seq++;

if ((seq % 24) == 0) {
currentPacket = currentPacket == whitePacket ? blackPacket : whitePacket;
}
unsigned long pt = time + 200;
currentPacket[0] = pt & 0xFF;
currentPacket[1] = (pt >> 8 ) & 0xFF;
currentPacket[2] = (pt >> 16) & 0xFF;
currentPacket[3] = (pt >> 24) & 0xFF;
currentPacket[4] = seq & 0xFF;
currentPacket[5] = 0; // flags











IPAddress ip = WiFi.localIP();


int res = udp.beginPacketMulticast(IPAddress(224, 0, 0, 116),
modules->configuration->Streaming->port,
ip,
1);
udp.write(currentPacket,PACKET_SIZE);
udp.endPacket();

}

}