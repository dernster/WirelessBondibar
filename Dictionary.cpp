#include "Dictionary.h"
#include <utility>


String& Dictionary::operator[](String key){
  for(int i = 0; i < pairs.size(); i++){
    if (pairs[i].first == key){
      return pairs[i].second; 
    }
  }
  pairs.push_back(StringPair(key,"dummy"));
  return pairAt(size()-1).second;
}
  
StringPair& Dictionary::pairAt(const int i){
  if (i < size()){
    return pairs[i];
  }
}

void Dictionary::append(Dictionary dict){
  pairs.insert(pairs.end(), dict.pairs.begin(), dict.pairs.end());
}

int Dictionary::size(){
  return pairs.size();
}

