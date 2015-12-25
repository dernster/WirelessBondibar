#include "ConfigurationServer.h"
#include <vector>
using namespace std;

SINGLETON_CPP(ConfigurationServer)

ConfigurationServer::ConfigurationServer(){
  configuration = singleton(Configuration);
  configuration->addObserver(this);
  buffer = new char[configuration->ConfigurationServer->packetLength];
  obtainServerEndpoint();
  udp.begin(configuration->ConfigurationServer->port);
  registerInServer();
}

bool ConfigurationServer::incomingCommand(){
  return udp.parsePacket() != 0;
}

void ConfigurationServer::processCommand(){
  int size = udp.read(buffer, configuration->ConfigurationServer->packetLength);
  buffer[size] = 0;
  String response(buffer);
  Dictionary params = parseParameters(response);
  configuration->setValues(params);
  Serial.println(configuration->toString());
  commandReceivedFlashLed();
}

void ConfigurationServer::registerInServer(){
  IPAddress ip = stringToIP(configuration->ConfigurationServer->ip);
  int res = udp.beginPacket(ip,configuration->ConfigurationServer->port);
  String data = String("=== REGISTER ===\n") +\
                "Device: " + String(configuration->Device->number) + "\n";
  udp.write(data.c_str(),data.length());
  udp.endPacket();
}

void ConfigurationServer::obtainServerEndpoint(){

  while(true){
    Serial.println("Discovering server...");
    // send broadcast packet asking IP and Port of server
    IPAddress ip = WiFi.localIP();
    ip[3] = 255;
    int res = udp.beginPacket(ip,configuration->ConfigurationServer->discoveryPort);
    String data = "Server IP and port?\nDevice: " + String(configuration->Device->number);
    udp.write(data.c_str(),data.length());
    udp.endPacket();
  
    // wait for response
    udp.begin(configuration->ConfigurationServer->discoveryPort);
    int maxWait = 2000; // ms
    while(maxWait){
      int size = udp.parsePacket();
      if (size){
          udp.read(buffer, size);
          buffer[size] = 0;
          String response(buffer);
          Dictionary params = parseParameters(response);
          configuration->setValues(params,false);
          Serial.println(configuration->toString());
          return;
      }
      delay(500);
      maxWait -= 500;
    }
  }
}

void ConfigurationServer::commandReceivedFlashLed(){
  flashLed(200,150,5);
}

ConfigurationServer::~ConfigurationServer(){
  delete []buffer;
  singleton(Configuration)->removeObserver(this);
}

void ConfigurationServer::configurationChanged(){
  Serial.println("ConfigurationServer::configurationChanged()");
  delete instance;
  instance = new ConfigurationServer();
}

