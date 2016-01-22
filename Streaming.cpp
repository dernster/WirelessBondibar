#include "Streaming.h"

SINGLETON_CPP(Streaming)

Streaming::Streaming(){
  configuration = singleton(Configuration);
  clock = singleton(TimeClock);
  configuration->addObserver(this);
  dataBuffer = NULL;
  setup();
}

void Streaming::setup(){
  bytesReceived = 0;
  udp.begin(configuration->Streaming->port);
  packetSize = configuration->Global->pixelsQty*3;
  if (dataBuffer){
    delete [] dataBuffer;
  }
  dataBuffer = new byte[packetSize];
  Serial.println(String("dataBuffer size = ") + String(packetSize));
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


Streaming::~Streaming(){
  if (dataBuffer)
    delete []dataBuffer;
  singleton(Configuration)->removeObserver(this);
}

void Streaming::configurationChanged(){
  Serial.println("Streaming::configurationChanged()");
  setup();
}

void Streaming::bufferFrame(){

  static short unsigned int expectedSeq = 0;

  int size = udp.read(dataBuffer, packetSize);
  bytesReceived += size + 8 + 20;

  time_r playbackTime = dataBuffer[0] + (dataBuffer[1]<<8) + (dataBuffer[2]<<16) + (dataBuffer[3]<<24);
  short int seq = dataBuffer[4] + (dataBuffer[5]<<8);

  Frame* frame = new Frame();
  frame->pt = playbackTime;
  frame->seq = seq;
  frame->len = configuration->Device->managedPixelsQty;
  int offset = 6 + configuration->Device->firstPixel;
  frame->data = copyBuffer(dataBuffer + offset, frame->len);

  if (frame->seq != expectedSeq){
    Serial.println("Wrong seq number! expected=" + String(expectedSeq) + " got=" + String(frame->seq));
  }

  expectedSeq = frame->seq + 1;

  if (buffer.size() >= 200){
    Serial.println("Buffer overloaded!");
    return;
  }

//  Serial.println("Current time: " + String(clock->time()));
//  Serial.println("Playback time: " + String(playbackTime));
  
  buffer.push_back(frame);
  updateBufferStat();
}

Frame* Streaming::frameToPlay(){

  static int times = 0;
  static float meanCount = 0;
  static float meanSum = 0;
  
  if (buffer.size() == 0)
    return NULL;
    
  time_r currentTime = clock->time();
  Frame* frame = buffer[0];
  time_r packetTime = frame->pt;

//  if ((currentTime >= 500) && (currentTime < 501)){
//    times++;
//  }else if (currentTime >= 501){
//    Serial.println("TImes = " + String(times));
//  }

  if (abs(currentTime - packetTime) <= 1){
    buffer.erase(buffer.begin());
    updateBufferStat();
    return frame;
  }else if(currentTime >= packetTime + 1){
    buffer.erase(buffer.begin());
    updateBufferStat();
    // Serial.println("Packet skipped! currentTime is ahead by: " + String(currentTime) + " - " + String(packetTime));
    meanCount++;
    meanSum += (currentTime - packetTime);
    configuration->Stats->playbackMeanDelay = meanSum / meanCount;
    if (configuration->Stats->playbackMaxDelay < currentTime - packetTime)
      configuration->Stats->playbackMaxDelay = currentTime - packetTime;    
  }

  return NULL;
}

void Streaming::updateBufferStat(){
  static unsigned long qty = 0;
  static unsigned long sizes = 0;

  qty++;
  sizes += buffer.size();
  
  configuration->Stats->streamingQueueMeanSize = (float)sizes/(float)qty;
}

