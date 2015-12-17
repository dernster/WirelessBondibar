#include "Streaming.h"

SINGLETON_CPP(Streaming)

Streaming::Streaming(){
  configuration = singleton(Configuration);
  buffer = NULL;
  configure();
}

void Streaming::configure(){
  udp.begin(configuration->streamingPort);
  packetSize = configuration->pixelsQty*3;
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

