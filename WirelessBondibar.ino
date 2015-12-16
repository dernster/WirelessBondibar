#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "Bondibar.h"

#define DEBUG(x)

#define PACKET_SIZE 24

int status = WL_IDLE_STATUS;
char ssid[] = "G2_6031";            //  your network SSID (name)
char pass[] = "27067243LB";         // your network password (use for WPA, or use as key for WEP)
unsigned int localPort = 7788;      // local port to listen on
byte packetBuffer[PACKET_SIZE];     // buffer to hold incoming packet

WiFiUDP Udp;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }


  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid,pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();

  Serial.println("\nStarting connection to server...");
  
  // if you get a connection, report back via serial:
  Udp.begin(localPort);
}

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

void loop() {

  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    DEBUG(
      Serial.print("Received packet of size ");
      Serial.println(packetSize);
      Serial.print("From ");
    )
    
    DEBUG(
      Serial.print(remoteIp);
      Serial.print(", port ");
      Serial.println(Udp.remotePort());
    )

    // read the packet into packetBufffer
    int len = Udp.read(packetBuffer, PACKET_SIZE);

    DEBUG(
      Serial.println("Contents:");
    )

    Bondibar.sendData(packetBuffer,PACKET_SIZE);
  }
}


