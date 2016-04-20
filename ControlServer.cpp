#include "ControlServer.h"
#include "APServer.h"
#include <vector>
#include "TimeClock.h"
#include "Streaming.h"
using namespace std;

extern "C" {
  #include "ets_sys.h"
  #include "os_type.h"
  #include "osapi.h"
  #include "mem.h"
  #include "user_interface.h"
  #include "cont.h"
}

SINGLETON_CPP(ControlServer)

ControlServer::ControlServer(){
  configuration = singleton(Configuration);
  configuration->addObserver(this);
  buffer = NULL;
  server = NULL;
  setup();
}

void ControlServer::setup(){
  lastPacketTime = millis();
  serverDiscovered = false;
  serverIsAlive = true;
  if (buffer)
    delete [] buffer;
  if (server) {
    delete server;
  }
  server = new WiFiServer(configuration->ControlServer->port);
  server->begin();
  buffer = new char[configuration->ControlServer->packetLength];
  obtainServerEndpoint();
}

bool ControlServer::incomingCommand(){
  bool packetReceived = false;

  if (client.connected()){
    packetReceived =  client.available();
  }else{
    client = server->available();
    packetReceived = client.connected() && client.available();
  }

  unsigned long actualTime = millis();
  if (packetReceived){
    lastPacketTime = actualTime;
    serverIsAlive = true;
  }else if (actualTime - lastPacketTime >= configuration->ControlServer->keepAliveSeconds*1000){
    serverIsAlive = false;
  }


  return packetReceived;
}

void ControlServer::externalCommandReceived(){
  lastPacketTime = millis();
}

SenderoControlHeader ControlServer::processCommand(){

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

  Serial.println(header.toString());


  if (header.requestClockFlag){

    /* we should review the first sync. If the offset is smaller than the
       calculated with the streaming packets, we could be in trouble.
       Because this first sync has the huge error of RTT */
    unsigned long currentTime = clock->rawTime();

    byte time[4];
    writeBuffer<unsigned long>(time,currentTime);

    client.write((uint8_t*)&header,header.size());
    client.write((uint8_t*)&time,sizeof(time));
  }

  if (header.requestStatsFlag){
    Serial.println("el ratee");
    Serial.println(configuration->Stats->bitRate);

    /* stats are dirty if streaming is active */
    configuration->Stats->dirty = singleton(Streaming)->active;

    String stats = configuration->Stats->toString();
    Serial.println("requestStatsFlag");
    Serial.println(stats);
    client.write((uint8_t*)&header,header.size());
    client.write((uint8_t*)stats.c_str(),stats.length() + 1);
  }


  if (header.clockCorrectionOffsetFlag){
    byte offsetBytes[4];
    client.readBytes((byte*)offsetBytes,4);
    long offset = readBuffer<long>(offsetBytes);
    // clock->addServerOffsetSample(offset, false,0,0,0);
  }




  if (header.configurationFlag){
    int i = 0;
    while(true){
      yield();
      byte b = client.read();
      if (b == -1){
        delay(100);
        continue;
      }
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
  }

  if (header.closeConnectionFlag){
    client.stop();
    Serial.println("Connection closed!");
  }

  return header;
}

void ControlServer::obtainServerEndpoint(){

  Serial.println("Discovering server on port " + String(configuration->ControlServer->discoveryPort) + "...");
  APServer* ap = singleton(APServer);

  while(true){

    yield();

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

      yield();

      /* allow connections from AP */
      ap->handleClient();

      if (client = server->available()){
        configuration->Streaming->serverIP = client.remoteIP().toString();
        Serial.printf("got client!, %s\n", configuration->Streaming->serverIP.c_str());

        Serial.println("waiting for configurations and clockSync...");

        /* receive initial configuration */
        bool configurationReceived = false;
        bool clockSync = false;

        while (!configurationReceived){
          yield();

          /* allow connections from AP */
          ap->handleClient();

          if (incomingCommand()){
            SenderoControlHeader h = processCommand();
            configurationReceived = configurationReceived || h.configurationFlag;
            clockSync = clockSync || h.clockCorrectionOffsetFlag;
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
