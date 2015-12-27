#include "APServer.h"

SINGLETON_CPP(APServer)

APServer::APServer() : server(80){
  Serial.print("Configuring access point...");
  WiFi.softAP("ESP");

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.on("/save", HTTP_POST,handleSave);
  server.begin();
  Serial.println("HTTP server started");
}

void APServer::handleRoot(){
  APServer* ap = singleton(APServer);
  ap->server.send(200, "text/html", buildPage());
}

void APServer::handleSave(){
  APServer* ap = singleton(APServer);
  ap->server.send(200, "text/html", "RESPONSE");
  for(int i = 0; i < ap->server.args(); i++){
    Serial.println(ap->server.argName(i) + ap->server.arg(i));
  }
}

String APServer::buildPage(){
  Configuration* conf = singleton(Configuration);
  String page1 = ""
""  
"<!DOCTYPE html>"
"<html>"
"<body>"
""
"<h2>AJAX</h2>"
""
"<script>"
"function loadDoc() {"
  "console.log(\"hola!\");"
  "var xhttp = new XMLHttpRequest();"
  "var div = document.getElementById(\"customForm\");"
  "var elms = div.getElementsByTagName(\"input\");"
  "console.log(elms);"
  "var data = \"\";"
  "for(var i = 0, maxI = elms.length; i < maxI; ++i){"
      "var elm = elms[i];"
      "console.log(elm.name + \" \" + elm.value);"
      "data += elm.name + \"=\" + elm.value + \"&\";"
  "}"
  "console.log(data);"
  "xhttp.onreadystatechange = function() {"
    "if (xhttp.readyState == 4 && xhttp.status == 200) {"
      "document.getElementById(\"demo\").innerHTML = xhttp.responseText;"
    "}"
  "};"
  "xhttp.open(\"POST\", \"http://192.168.4.1/save\", true);"
  "xhttp.setRequestHeader(\"Content-type\", \"application/x-www-form-urlencoded\");"
  "xhttp.send(data);"
"}"
"</script>"
"<div id=\"customForm\">";

  Dictionary dict = conf->toDictionary();
  String inputs = "";

  for(int i = 0; i < dict.size(); i++){
    StringPair& pair = dict.pairAt(i);
    inputs += pair.first + ": <input type=\"text\" name=\"" + pair.first +"\" value=\"" + pair.second + "\"><br>";
  }

String page2 = ""
""
"</div>"
""
"<button type=\"button\" onclick=\"loadDoc()\">Request data</button>"
"</body>"
"</html>"
;

  return page1 + inputs + page2;
}

