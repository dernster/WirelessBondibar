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
  udp.begin(configuration->Streaming->port);
  packetSize = configuration->Global->pixelsQty*3 + 6; /* 6 = timestamp + seqNumber */
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
    singleton(ControlServer)->externalCommandReceived();
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

//=======================================

// #define OFFSETS_QTY (24*15)
// long offsets[OFFSETS_QTY] = {0};
// int offIndex = 0;
// double SMA = 0;
double CMA = 0, prevCMA = 0;

double initialCum = 0;


//=======================================

void Streaming::bufferFrame(){

  static bool firstFrame = true;
  static short unsigned int expectedSeq = 0;
  static unsigned long totalPackets = 0;
  static unsigned long lostPackets = 0;


  int size = udp.read(dataBuffer, packetSize);
  bytesReceived += size + 8 + 20;

  time_r playbackTime = dataBuffer[0] + (dataBuffer[1]<<8) + (dataBuffer[2]<<16) + (dataBuffer[3]<<24);
  short int seq = dataBuffer[4] + (dataBuffer[5]<<8);

  time_r serverTime = playbackTime - 240;

  time_r myTime = clock->rawTime();
  long offset = serverTime - myTime;

  prevCMA = CMA;
  CMA = (double)offset/((double)(totalPackets+1)) + (double)(totalPackets*prevCMA)/(double)(totalPackets+1);
  long LCMA = round(CMA);
  clock->setCorrection(LCMA);

  // long oldOffset = offsets[offIndex];
  // offsets[offIndex] = offset;
  // offIndex = (offIndex + 1) % OFFSETS_QTY;
  //
  // if (totalPackets < OFFSETS_QTY) {
  //   clock->setCorrection(offset);
  //   initialCum += offset;
  // } else {
  //   if (totalPackets == OFFSETS_QTY){
  //     SMA = initialCum/(double)OFFSETS_QTY;
  //   }
  //
  //   SMA = SMA + (double)offset/(double)OFFSETS_QTY - (double)oldOffset/(double)OFFSETS_QTY;
  //   long LSMA = round(SMA);
  //   clock->setCorrection(LSMA);

   if ((totalPackets % 24*5) == 0){
     Serial.println(LCMA);
   }
  // }

  totalPackets++;
  Frame* frame = new Frame();
  frame->pt = playbackTime;
  frame->seq = seq;
  frame->len = configuration->Device->managedPixelsQty;
  int bufferOffset = 6 + configuration->Device->firstPixel;
  frame->data = copyBuffer(dataBuffer + bufferOffset, frame->len);


  if (!firstFrame && (frame->seq != expectedSeq)){
//    Serial.println("Wrong seq number! expected=" + String(expectedSeq) + " got=" + String(frame->seq));
    lostPackets++;
    configuration->Stats->packetLossRate = (float)lostPackets/((float)totalPackets);
  }

  expectedSeq = frame->seq + 1;

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

  time_r currentTime = clock->time();
  Frame* frame = buffer[0];
  time_r packetTime = frame->pt;

  if (abs(currentTime - packetTime) <= 0){
    times++;
    buffer.erase(buffer.begin());
    updateBufferStat();
  }else if(currentTime >= packetTime + 1){
    Serial.println("Frame delayed!!!!!");
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
