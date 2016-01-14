#include "ControlServer.h"
#include "APServer.h"
#include <vector>
using namespace std;

SINGLETON_CPP(ControlServer)

ControlServer::ControlServer(){
  configuration = singleton(Configuration);
  configuration->addObserver(this);
  buffer = NULL;
  setup();
}

void ControlServer::setup(){
  serverDiscovered = false;
  if (buffer)
    delete [] buffer;
  if (server)
    delete server;
  server = new WiFiServer(configuration->ControlServer->port);
  server->begin();
  buffer = new char[configuration->ControlServer->packetLength];
  obtainServerEndpoint();
}

bool ControlServer::incomingCommand(){
  return client.available();
}

void ControlServer::processCommand(){
  SenderoControlHeader header;
  Serial.println(header.size());
  client.readBytes((byte*)&header,header.size());
  Serial.println(header.toString());
  commandReceivedFlashLed();
}

void ControlServer::obtainServerEndpoint(){

  

  Serial.println("Discovering server on port " + String(configuration->ControlServer->discoveryPort) + "...");
  APServer* ap = singleton(APServer);
  
  while(true){
    
    /* allow connections from AP */
    ap->handleClient();
        
    // send broadcast packet asking IP and Port of server
    IPAddress ip = WiFi.localIP();
    ip[3] = 255;
    int res = udp.beginPacket(ip,configuration->ControlServer->discoveryPort);
    String data = "Device: " + String(configuration->Device->number);
    udp.write(data.c_str(),data.length());
    udp.endPacket();
  
    // wait for response
 
    LOOP_UNTIL(2000){
      /* allow connections from AP */
      ap->handleClient();
      if (!client.connected()){
        client = server->available();
      }else{
        Serial.println("got client!");
        return;
      }
    }
  }
}

void ControlServer::commandReceivedFlashLed(){
  flashLed(200,150,5);
}

ControlServer::~ControlServer(){
  if (buffer)
    delete []buffer;
  singleton(Configuration)->removeObserver(this);
}

void ControlServer::configurationChanged(){
  Serial.println("ControlServer::configurationChanged()");
  setup();
}

