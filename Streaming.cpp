#include "Streaming.h"

SINGLETON_CPP(Streaming)

Streaming::Streaming(){
  configuration = singleton(Configuration);
  configuration->addObserver(this);
  buffer = NULL;
  setup();
}

void Streaming::setup(){
  bytesReceived = 0;
  udp.begin(configuration->Streaming->port);
  packetSize = configuration->Global->pixelsQty*3;
  if (buffer){
    delete [] buffer;
  }
  buffer = new byte[packetSize];
  Serial.println(String("Buffer size = ") + String(packetSize));
  active = true;
}

bool Streaming::frame(){
  bool packetIsThere = udp.parsePacket(); 
  bool lastActive = active;
  unsigned long actualTime;
  
  /* active-inactive update */
  static unsigned long lastPacketTime = 0;
  if (packetIsThere){
    lastPacketTime = millis();
    active = true;
  }else{
    actualTime = millis();
    active = ((actualTime - lastPacketTime) < 5*1000); /* 5 seconds */
  }
  if (lastActive && !active){
    Serial.println("Streaming is now INACTIVE!");
    stop = millis() - (actualTime - lastPacketTime);

    /* update configuration */
    configuration->Stats->bitRate = (8*bytesReceived/((stop - start)/1000.0))/1000.0;

  }else if (!lastActive && active){
    start = lastPacketTime;
    bytesReceived = 0;
    Serial.println("Streaming is now ACTIVE!");
  }
  
  return packetIsThere;
}

void Streaming::readFrame(){
  int size = udp.read(buffer, packetSize);
  bytesReceived += size + 8 + 20;
}

Streaming::~Streaming(){
  if (buffer)
    delete []buffer;
  singleton(Configuration)->removeObserver(this);
}

void Streaming::configurationChanged(){
  Serial.println("Streaming::configurationChanged()");
  setup();
}

