#include "ControlServer.h"
#include "APServer.h"
#include <vector>
#include "TimeClock.h"
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

  TimeClock* clock = singleton(TimeClock);

  // parse command
  
  if (header.requestClockFlag){

    time_r currentTime = clock->time();
    header.isResponse = true;

    byte time[4];
    writeBuffer<time_r>(time,currentTime);

    client.write((uint8_t*)&header,header.size());
    client.write((uint8_t*)&time,sizeof(time)); 
    
  }else if (header.clockCorrectionOffsetFlag){

    byte offsetBytes[4];
    client.readBytes((byte*)offsetBytes,4);
    long offset = readBuffer<long>(offsetBytes);
    Serial.println(String("offset received = ") + String(offset));
    clock->addCorrection(offset);
    Serial.println(String("time adjsted ") + String(clock->time()));
  }

  
//  commandReceivedFlashLed();
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


