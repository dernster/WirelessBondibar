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

void connectToWifi(){
  WiFi.disconnect();
  Configuration* configuration = singleton(Configuration);
  WiFi.begin(configuration->ssid.c_str(),configuration->password.c_str());
  while ( WiFi.status() != WL_CONNECTED) {
    Serial.println("Attempting to connect to SSID: " + configuration->ssid);
    delay(500);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();
}

String ipToString(IPAddress ip){
  String res = String(ip[0]) + "." +\
  String(ip[1]) + "." +\
  String(ip[2]) + "." +\
  String(ip[3]);
  return res;
}

