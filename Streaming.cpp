#include "Streaming.h"

SINGLETON_CPP(Streaming)

Streaming::Streaming(){
  configuration = singleton(Configuration);
  configuration->addObserver(this);
  buffer = NULL;
  configure();
}

void Streaming::configure(){
  udp.begin(configuration->Streaming->port);
  packetSize = configuration->Global->pixelsQty*3;
  if (buffer != NULL){
    delete [] buffer;
  }
  buffer = new byte[packetSize];
}


bool Streaming::frame(){
  return udp.parsePacket() != 0;
}

void Streaming::readFrame(){
  udp.read(buffer, packetSize);
}

Streaming::~Streaming(){
  delete []buffer;
  singleton(Configuration)->removeObserver(this);
}

void Streaming::configurationChanged(){
  Serial.println("Streaming::configurationChanged()");
  delete instance;
  instance = new Streaming();
}

