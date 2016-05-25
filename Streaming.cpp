#include "Streaming.h"
#include "ControlServer.h"

SINGLETON_CPP(Streaming)

#define STREAMING_HEADER_SIZE  (6)

Streaming::Streaming(){
  configuration = singleton(Configuration);
  clock = singleton(TimeClock);
  configuration->addObserver(this);
  dataBuffer = NULL;
  lastFramesBuffer = std::vector<Frame*> (FRAMES_TO_INTERPOLATE);
  lastFrameIndex = -1;
  for (int i = 0; i < FRAMES_TO_INTERPOLATE; i++)
    lastFramesBuffer[i] = NULL;
  setup();
}

void Streaming::setup(){
  bytesReceived = 0;
  lastArrivedPacketTimestamp = 0;
  // Serial.println(configuration->Streaming->multicastGroupIp);
  udp.beginMulticast(WiFi.localIP(), stringToIP(configuration->Streaming->multicastGroupIp), 7788);
  // Serial.printf("configuration->Streaming->pixelsQty %i\n", configuration->Streaming->pixelsQty);
  // Serial.printf("configuration->Streaming->multicastGroupFirstPixel %i\n", configuration->Streaming->multicastGroupFirstPixel);
  packetSize = configuration->Streaming->pixelsQty*3 + STREAMING_HEADER_SIZE; /* 6 = timestamp + seqNumber + flags */
  if (dataBuffer){
    delete [] dataBuffer;
  }
  dataBuffer = new byte[packetSize];
  // Serial.println(String("dataBuffer size = ") + String(packetSize));
  active = true;
}

bool Streaming::frame(){
  int packetIsThere = udp.parsePacket();

  if (packetIsThere){
    /* Every packet gets timestamped instantly when arrives.
       Later on, the offset with server is calculated using this arrival timestamp. */
    lastArrivedPacketTimestamp = clock->rawTime();

    // if(udp.remoteIP().toString() != configuration->Streaming->serverIP){
    //   udp.flush();
    //   return false;
    // }
    if (packetIsThere != packetSize){
      udp.flush();
      // Serial.printf("Wrong size of streaming packet! %i\n",packetIsThere);
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
    // Serial.println("Streaming is now INACTIVE!");
    stop = lastPacketTime;

    /* update configuration */
    configuration->Stats->bitRate = (8*bytesReceived/((stop - start)/1000.0))/1000.0;

  }else if (!lastActive && active){
    start = lastPacketTime;
    bytesReceived = 0;
    // Serial.println("Streaming is now ACTIVE!");
  }

  return packetIsThere;
}

void Streaming::bufferFrame(){
  static bool waitingForSyncFrame = true;
  static bool firstFrame = true;
  static byte expectedSeq = 0;
  static unsigned long totalReceivedPackets = 0;
  static unsigned long lostPackets = 0;

  int size = udp.read(dataBuffer, packetSize);
  bytesReceived += size + 8 + 20 + 14;

  unsigned long playbackTime = dataBuffer[0] + (dataBuffer[1]<<8) + (dataBuffer[2]<<16) + (dataBuffer[3]<<24);
  byte seq = dataBuffer[4];
  byte flags = dataBuffer[5];

  Frame* frame = new Frame();
  frame->pt = playbackTime;
  frame->seq = seq;
  frame->flags = flags;
  frame->len = configuration->Device->managedPixelsQty*3;
  frame->arriveTime = lastArrivedPacketTimestamp;
  int offset = STREAMING_HEADER_SIZE + configuration->Device->firstPixel*3 - configuration->Streaming->multicastGroupFirstPixel*3;
  frame->data = copyBuffer(dataBuffer + offset, frame->len);

  bool isValid = clock->updateServerOffset(frame);

  if (!isValid){
    delete frame;
    return;
  }

  totalReceivedPackets++;
  if (!firstFrame && (frame->seq != expectedSeq)){
    // Serial.println("Wrong seq number! expected=" + String(expectedSeq) + " got=" + String(frame->seq));
    if (lastFrameIndex > -1) {
      int i;
      int cfi = lastFrameIndex;
      for (i = expectedSeq; i == frame->seq - 1; i = (i + 1) % 256) {

        Frame* lastReceivedFrame = lastFramesBuffer[cfi];

        Frame* customFrame = new Frame();
        customFrame->pt = lastReceivedFrame->pt + (frame->pt - lastReceivedFrame->pt) / (abs(frame->seq - lastReceivedFrame->seq) % 256);
        customFrame->seq = expectedSeq;
        customFrame->flags = frame->flags;
        customFrame->len = configuration->Device->managedPixelsQty*3;
        customFrame->arriveTime = clock->rawTime();
        customFrame->data = new byte[customFrame->len];

        for (int j = 0; j < customFrame->len; j = j + 3) {
          customFrame->data[j] = (lastReceivedFrame->data[j] + frame->data[j]) / 2;
          customFrame->data[j+1] = (lastReceivedFrame->data[j+1] + frame->data[j+1]) / 2;
          customFrame->data[j+2] = (lastReceivedFrame->data[j+2] + frame->data[j+2]) / 2;
        }

        buffer.push_back(customFrame);
        updateBufferStat();

        lastFrameIndex = cfi;
        cfi = (cfi + 1) % FRAMES_TO_INTERPOLATE;

      }
    }
    lostPackets += frame->seq - expectedSeq;
    configuration->Stats->packetLossRate = (float)lostPackets / ((float)totalReceivedPackets + (float)lostPackets);
  }

  expectedSeq = (frame->seq + 1) % 256;

  if (buffer.size() >= 200){
    // Serial.println("Buffer overloaded!");
    return;
  }

  lastFrameIndex = (lastFrameIndex + 1) % FRAMES_TO_INTERPOLATE;

  if (lastFramesBuffer[lastFrameIndex] != NULL)
    delete lastFramesBuffer[lastFrameIndex];

  lastFramesBuffer[lastFrameIndex] = new Frame(*frame);

  firstFrame = false;
  buffer.push_back(frame);
  updateBufferStat();
}

Frame* Streaming::frameToPlay(){

  static int times = 0;
  static float delaysCount = 0;
  static float delaysSum = 0;
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
  } else if U32_TIME_GREATER_THAN(currentTime, packetTime + 1){
    // Serial.println("Frame delayed!!");
    times++;
    delayedPackets++;
    buffer.erase(buffer.begin());
    updateBufferStat();
    delaysCount++;
    delaysSum += (currentTime - packetTime);
    configuration->Stats->playbackMeanDelay = delaysSum / delaysCount;
    if (currentTime - packetTime > configuration->Stats->playbackMaxDelay)
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

Streaming::~Streaming(){
  if (dataBuffer)
    delete []dataBuffer;
  singleton(Configuration)->removeObserver(this);
}

void Streaming::configurationChanged(){
  // Serial.println("Streaming::configurationChanged()");
  setup();
}
