#include "Streaming.h"

SINGLETON_CPP(Streaming)

Streaming::Streaming(){
  configuration = singleton(Configuration);
  configuration->addObserver(this);
  buffer = NULL;
  setup();
}

void Streaming::setup(){
  udp.begin(configuration->Streaming->port);
  packetSize = configuration->Global->pixelsQty*3;
  if (buffer){
    delete [] buffer;
  }
  buffer = new byte[packetSize];
  active = true;
}

bool Streaming::frame(){
  bool packetIsThere = udp.parsePacket(); 
  bool lastActive = active;

  /* active-inactive update */
  static unsigned long lastPacketTime = 0;
  if (packetIsThere){
    lastPacketTime = millis();
    active = true;
  }else{
    unsigned long actualTime = millis();
    active = ((actualTime - lastPacketTime) < 5*1000); /* 5 seconds */
  }
  if (lastActive && !active){
    Serial.println("Streaming is now INACTIVE!");
  }else if (!lastActive && active){
    Serial.println("Streaming is now ACTIVE!");
  }
  
  return packetIsThere;
}

void Streaming::readFrame(){
  udp.read(buffer, packetSize);
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

