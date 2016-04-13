#pragma once
#include "Configuration.h"

template <typename T>
struct Sample{
  T sample;
  unsigned long timestamp;

  Sample(){
    sample = 0;
    timestamp = 0;
  }

  Sample(const T & sample, const unsigned long & timestamp){
    this->sample = sample;
    this->timestamp = timestamp;
  }
};

class Frame{
public:
  Configuration* conf;
  byte* data;
  int len;
  unsigned long pt;
  byte seq;
  byte flags;
  unsigned long arriveTime;

  Frame(){
    conf = singleton(Configuration);
  }

  Frame(const Frame & frame) {
    this->conf = singleton(Configuration);
    this->len = frame.len;
    this->data = new byte[this->len];
    for (int i = 0; i < this->len; i++)
      this->data[i] = frame.data[i];
    this->seq = frame.seq;
    this->flags = frame.flags;
    this->pt = frame.pt;
    this->arriveTime = frame.arriveTime;
  }

  Sample<long> getOffsetAgainstServerTime(){
    long sample = (pt - conf->Streaming->playbackTimeDelay) - arriveTime;
    return Sample<long>(sample, arriveTime);
  }

  ~Frame(){
    if (data)
      delete [] data;
  }
};
