#pragma once
#include <vector>
#include "Arduino.h"
using namespace std;

typedef std::pair<String, String> StringPair;

class Dictionary{
private:
  vector<StringPair> pairs;
public:
  String& operator[](String key);
  StringPair& pairAt(const int i);
  void append(Dictionary dict);
  String toString();
  int size(); 
};

