#include "Streaming.h"
#include "ControlServer.h"

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
  lastArrivedPacketTimestamp = 0;
  udp.begin(configuration->Streaming->port);
  packetSize = configuration->Global->pixelsQty*3 + 6; /* 6 = timestamp + seqNumber + flags */
  if (dataBuffer){
    delete [] dataBuffer;
  }
  dataBuffer = new byte[packetSize];
  Serial.println(String("dataBuffer size = ") + String(packetSize));
  active = true;
}

bool Streaming::frame(){
  int packetIsThere = udp.parsePacket();

  if (packetIsThere){

    /* Every packet gets timestamped instantly when arrives.
       Later on, the offset with server is calculated using this arrival timestamp. */
    lastArrivedPacketTimestamp = clock->rawTime();

    if(udp.remoteIP().toString() != configuration->Streaming->serverIP){
      udp.flush();
      // Serial.printf("Wrong IP! %s, %s\n", udp.remoteIP().toString().c_str(), configuration->Streaming->serverIP.c_str());
      return false;
    }
    if (packetIsThere != packetSize){
      udp.flush();
      Serial.printf("Wrong size of streaming packet! %i\n",packetIsThere);
      return false;
    }
  }

  bool lastActive = active;
  unsigned long actualTime;

  /* active-inactive update */
  static time_t lastPacketTime = 0;
  if (packetIsThere){
    lastPacketTime = clock->rawTime();
    singleton(ControlServer)->externalCommandReceived();
    active = true;
  }else{
    actualTime = clock->rawTime();
    active = ((actualTime - lastPacketTime) < 5*1000); /* 5 seconds */
  }
  if (lastActive && !active){
    Serial.println("Streaming is now INACTIVE!");
    stop = clock->rawTime() - (actualTime - lastPacketTime);

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
  static bool waitingForSyncFrame = true;
  static bool firstFrame = true;
  static byte expectedSeq = 0;
  static unsigned long totalPackets = 0;
  static unsigned long lostPackets = 0;

  int size = udp.read(dataBuffer, packetSize);
  bytesReceived += size + 8 + 20;

  unsigned long playbackTime = dataBuffer[0] + (dataBuffer[1]<<8) + (dataBuffer[2]<<16) + (dataBuffer[3]<<24);
  byte seq = dataBuffer[4];
  byte flags = dataBuffer[5];

  /* update server offset statistics */
  Frame* frame = new Frame();
  frame->pt = playbackTime;
  frame->seq = seq;
  frame->expClkSync = (flags & 0x01 == 0x01);
  frame->len = configuration->Device->managedPixelsQty*3;

  unsigned long serverTime = playbackTime - 200;

  // Serial.printf("This is the flags mask: %i and the boolean: %i\n", flags, (flags & 0x01 == 0x01));
  clock->addServerOffsetSample((long)(serverTime - lastArrivedPacketTimestamp), frame->expClkSync);

  // if ((waitingForSyncFrame) && ((seq % (24*5)) == 0)){
  //   waitingForSyncFrame = false;
  // }
  //
  // if (waitingForSyncFrame)
  //   return;

  int offset = 5 + configuration->Device->firstPixel*3;
  frame->data = copyBuffer(dataBuffer + offset, frame->len);

  totalPackets++;
  if (!firstFrame && (frame->seq != expectedSeq)){
   Serial.println("Wrong seq number! expected=" + String(expectedSeq) + " got=" + String(frame->seq));
    lostPackets++;
    configuration->Stats->packetLossRate = (float)lostPackets/((float)totalPackets);
  }

  expectedSeq = (frame->seq + 1) % 256;

  if (buffer.size() >= 200){
    Serial.println("Buffer overloaded!");
    return;
  }

  firstFrame = false;
  buffer.push_back(frame);
  updateBufferStat();
}

Frame* Streaming::frameToPlay(){

  static int times = 0;
  static float meanCount = 0;
  static float meanSum = 0;
  static int delayedPackets = 0;

  if (buffer.size() == 0)
    return NULL;

  unsigned long currentTime = clock->time();
  Frame* frame = buffer[0];
  unsigned long packetTime = frame->pt;

  if (abs((long)(currentTime - packetTime)) <= 1){
    times++;
    buffer.erase(buffer.begin());
    updateBufferStat();
  }else if(currentTime >= packetTime + 1){
    Serial.println("Frame delayed!!");
    times++;
    delayedPackets++;
    buffer.erase(buffer.begin());
    updateBufferStat();
    meanCount++;
    meanSum += (currentTime - packetTime);
    configuration->Stats->playbackMeanDelay = meanSum / meanCount;
    if (configuration->Stats->playbackMaxDelay < (currentTime - packetTime))
      configuration->Stats->playbackMaxDelay = currentTime - packetTime;

    delete frame;
    frame = NULL;
  }else {
    frame = NULL;
  }

  configuration->Stats->delayedFramesRate = ((float)delayedPackets)/((float)times);
  return frame;
}

void Streaming::updateBufferStat(){
  static unsigned long qty = 0;
  static unsigned long sizes = 0;

  qty++;
  sizes += buffer.size();

  configuration->Stats->streamingQueueMeanSize = (float)sizes/(float)qty;

  if (buffer.size() > configuration->Stats->streamingQueueMaxSize){
    configuration->Stats->streamingQueueMaxSize = buffer.size();
  }
}
