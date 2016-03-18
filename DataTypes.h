#pragma once

class Frame{
public:
  byte* data;
  int len;
  unsigned long pt;
  byte seq;
  bool expClkSync;
  unsigned long arriveTime;

  ~Frame(){
    if (data)
      delete [] data;
  }
};

struct OffsetSample {
  double sample;
  unsigned long timestamp;

  OffsetSample(){
    sample = 0;
    timestamp = 0;
  }

  OffsetSample(const double & sample, const unsigned long & timestamp){
    this->sample = sample;
    this->timestamp = timestamp;
  }
};
