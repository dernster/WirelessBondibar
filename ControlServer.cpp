#include "ControlServer.h"
#include "APServer.h"
#include <vector>
#include "TimeClock.h"
using namespace std;

SINGLETON_CPP(ControlServer)

ControlServer::ControlServer(){
  Serial.println("1");
  configuration = singleton(Configuration);
  Serial.println("2");
  configuration->addObserver(this);
  Serial.println("3");
  buffer = NULL;
  server = NULL;
  setup();
}

void ControlServer::setup(){
  Serial.println("4");
  serverDiscovered = false;
  if (buffer)
    delete [] buffer;
  Serial.println("5");
  if (server)
    delete server;
  Serial.println("6");
  server = new WiFiServer(configuration->ControlServer->port);
  Serial.println("7");
  server->begin();
  Serial.println("8");
  buffer = new char[configuration->ControlServer->packetLength];
  Serial.println("9");
  obtainServerEndpoint();
  Serial.println("10");
}

bool ControlServer::incomingCommand(){
  if (client.connected()){
    return client.available();
  }else{
    client = server->available();
    return client.connected() && client.available();
  }
}

SenderoControlHeader::Command ControlServer::processCommand(){
  if (!client.connected()){
    Serial.println("NO CONNECTION!!!");
  }
  
  SenderoControlHeader header;
  int qty = client.readBytes((byte*)&header,header.size());
  if (qty != header.size()){
    Serial.println("Lei menos del cabezal!!!!");
  }

  SenderoControlHeader::Command command = header.type();

  TimeClock* clock = singleton(TimeClock);

  // parse command

  switch (command) {
  
  case SenderoControlHeader::Command::REQUEST_CLOCK:{

    time_r currentTime = clock->time();

    byte time[4];
    writeBuffer<time_r>(time,currentTime);

    client.write((uint8_t*)&header,header.size());
    client.write((uint8_t*)&time,sizeof(time)); 
    
    break;
  }
  case SenderoControlHeader::Command::CLOCK_CORRECTION:{

    byte offsetBytes[4];
    client.readBytes((byte*)offsetBytes,4);
    long offset = readBuffer<long>(offsetBytes);
    clock->addCorrection(offset);
    break;
  }
  case SenderoControlHeader::Command::CONFIGURATION:{

    int i = 0;
    while(true){
      byte b = client.read();
      if (b == -1)
        continue;

      buffer[i] = (char)b;
      
      if (buffer[i] == '\0'){    
        break;
      }
      i++;
    }

    String configs = String(buffer);
    Dictionary dict = parseParameters(configs);
    configuration->setValues(dict,false);

    Serial.println(String("Configs setted! ") + configs);
    break;
  }
  }

  if (header.closeConnectionFlag){
    client.stop();
    Serial.println("Connection closed!");
  }

  return header.type();
}

void ControlServer::obtainServerEndpoint(){

  

  Serial.println("Discovering server on port " + String(configuration->ControlServer->discoveryPort) + "...");
  APServer* ap = singleton(APServer);
  
  while(true){
    
    /* allow connections from AP */
    ap->handleClient();
        
    // send broadcast packet asking IP and Port of server
    Serial.println("Sending message to register");
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
      if (client = server->available()){
        Serial.println("got client!");

        Serial.println("waiting for configurations and clockSync...");

        /* receive initial configuration */
        bool configurationReceived = false;
        bool clockSync = false;
        while (!configurationReceived || !clockSync){
          if (incomingCommand()){
            SenderoControlHeader::Command type = processCommand();
            configurationReceived = type == SenderoControlHeader::Command::CONFIGURATION;
            clockSync = type == SenderoControlHeader::Command::CLOCK_CORRECTION;
          }
        }

        Serial.println("Going for streaming!");
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

template<typename T> T ControlServer::readBuffer(void* buffer){
  int size = sizeof(T);
  T result = 0;
  for(int i = 0; i < size; i++){
    byte b = ((byte*)buffer)[i];
    result |= (b << (i*8));
  }
  return result;
}

template<typename T> void ControlServer::writeBuffer(void* buffer, T data){
  int size = sizeof(T);
  for(int i = 0; i < size; i++){
    ((byte*)buffer)[i] = (byte)((data >> (i*8)) & 0xFF);
  }
}


