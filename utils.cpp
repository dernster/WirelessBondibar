#include "utils.h"


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}


String ipToString(IPAddress ip){
  String res = String(ip[0]) + "." +\
  String(ip[1]) + "." +\
  String(ip[2]) + "." +\
  String(ip[3]);
  return res;
}

IPAddress stringToIP(String ipStr){
  vector<String> numbers = splitString(ipStr,'.');
  int n[4];
  for(int i = 0; i < 4; i++){
    n[i] = numbers[i].toInt();
  }
  return IPAddress(n[0],n[1],n[2],n[3]);
}

vector<String> splitString(String string, char delimiter){
  vector<String> result;
  int lastStart = 0;
  for(int i = 0; i < string.length(); i++){
    if (string.charAt(i) == delimiter){
      result.push_back(string.substring(lastStart,i));
      lastStart = i+1;
    }
  }
  if (lastStart < string.length()){
    result.push_back(string.substring(lastStart,string.length()));
  }
  return result;
}

Dictionary parseParameters(String stringParams){
  Dictionary result;
  vector<String> params = splitString(stringParams,' ');
  for(int i = 0; i < params.size(); i++){
    vector<String> keyValue = splitString(params[i],':');
    if (keyValue.size() == 2){
      result[keyValue[0]] = keyValue[1];
    }
  }
  return result;
}

void flashLed(int onMs, int offMs, int times){
  for(int i = 0; i < times; i++){
    digitalWrite(LED,LOW);
    delay(onMs);
    digitalWrite(LED,HIGH);
    delay(offMs);
  }
}

byte* copyBuffer(byte* src,int len){
  byte* dst = new byte[len];
  for(int i = 0; i < len; i++)
    dst[i] = src[i];
  return dst;
}

bool msIsMultiple(unsigned long time, unsigned long multiple) {
  static bool alreadyEntered = false;
  if ((!alreadyEntered) && ((time % multiple) == 0)){
    alreadyEntered = true;
    return true;
  } else if ((time % multiple) != 0){
    alreadyEntered = false;
  }
  return false;
}
