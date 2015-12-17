#include "ConfigurationServer.h"

SINGLETON_CPP(ConfigurationServer)

ConfigurationServer::ConfigurationServer(){
  configuration = singleton(Configuration);
  becomeVisibleToNetwork();
  buffer = new char[configuration->commandPacketLength];
  udp.begin(configuration->commandsPort);
}

bool ConfigurationServer::incomingCommand(){
  return udp.parsePacket() != 0;
}

void ConfigurationServer::processCommand(){
  udp.read(buffer, configuration->commandPacketLength);
  buffer[configuration->commandPacketLength-1] = 0;
  // parse command
  Serial.println(buffer);
}

void ConfigurationServer::becomeVisibleToNetwork(){
  IPAddress ip = WiFi.localIP();
  ip[3] = 255;
  int res = udp.beginPacket(ip,configuration->commandsPort);
  String data = "Device: " + String(configuration->deviceNumber) + "\n" +\
                "IP: " + ipToString(WiFi.localIP()) + "\n";
  Serial.println(data);
  Serial.println(data.length());
  udp.write(data.c_str(),data.length());
  udp.endPacket();
}

