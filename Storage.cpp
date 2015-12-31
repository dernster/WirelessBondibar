#include "Storage.h"
#include <EEPROM.h>

SINGLETON_CPP(Storage)

byte Storage::readDeviceID(){
  EEPROM.begin(1);
  byte id = EEPROM.read(0);
  EEPROM.end();
  return id;
}

void writeBytes(int addr,const char* buffer, int size){
  int i = 0;
  while(i < size){
    EEPROM.write(addr,buffer[i]);
    addr++;
    i++;
  }
}

void Storage::readSSIDAndPassword(String& ssid, String& password){
  EEPROM.begin(200);
  ssid = "";
  password = "";
  char r;
  int addr = 1;
  while ((r = char(EEPROM.read(addr))) != '\0'){
    ssid += String(r);
    addr++;
  }
  addr++;
  while ((r = char(EEPROM.read(addr))) != '\0'){
    password += String(r);
    addr++;
  }
  EEPROM.end();
}
void Storage::setSSIDAndPassword(String ssid, String password){
  EEPROM.begin(200);
  writeBytes(1,ssid.c_str(),ssid.length()+1);
  writeBytes(1 + ssid.length() + 1, password.c_str(), password.length()+1);
  EEPROM.end();
}

