#pragma once
#include "Configuration.h"
#include <vector>

using namespace std;

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
  bool isGeneratedFrame;
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
    this->isGeneratedFrame = frame.isGeneratedFrame;
  }

  Sample<long> getOffsetAgainstServerTime(){
    long sample = (pt - conf->Streaming->playbackTimeDelay) - arriveTime;
    return Sample<long>(sample, arriveTime);
  }

  static void insertFrameInOrder(vector<Frame *> &buffer, Frame *frame) {
    /****
     ** Inserts ordered by pt
     */
    if (buffer.size() > 0) {

      int i = buffer.size() - 1;
      while (i >= 0 && U32_TIME_GREATER_THAN(buffer[i]->pt, frame->pt)) {
        // Serial.printf("el i: %i\n", i);
        i--;
      }

      // if (buffer[i]->seq == frame->seq)
      //   Serial.printf("EL MISMO PT!!!! %lu\n", frame->seq);

      if (i == buffer.size() - 1)
        buffer.push_back(frame);
      else
        buffer.insert(buffer.begin() + i + 1, frame);

    } else
      buffer.push_back(frame);
  }

  ~Frame(){
    if (data)
      delete [] data;
  }
};
