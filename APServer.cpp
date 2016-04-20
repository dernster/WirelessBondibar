#include "APServer.h"
#include <ESP8266mDNS.h>

SINGLETON_CPP(APServer)

String APServer::apIP = "";

APServer::APServer(){
  Serial.print("Configuring access point...");
  Configuration* conf = singleton(Configuration);
  server = NULL;
  setup();
}

void APServer::setup(){
  if (server)
    delete server;
  Configuration* conf = singleton(Configuration);
  server = new ESP8266WebServer(80);
  String n = String(conf->Device->number);
  apIP = "192.168.4.1";
  WiFi.softAP(("WBB-" + n).c_str());

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server->on("/", handleRoot);
  server->on("/save", HTTP_POST,handleSave);
  server->begin();
  Serial.println("HTTP server started");
}

void APServer::handleClient(){
  server->handleClient();
}

void APServer::handleRoot(){
  Serial.println("Page requested!");
  APServer* ap = singleton(APServer);
  ap->server->send(200, "text/html", buildPage());
}

void APServer::handleSave(){
  APServer* ap = singleton(APServer);
  ap->server->send(200, "text/html", "Ok");

  Dictionary params;
  for(int i = 0; i < ap->server->args(); i++){
    params[ap->server->argName(i)] = ap->server->arg(i);
  }

  if (params.size() > 0){
    Serial.println("Configs saved!");
    singleton(Configuration)->setValues(params);
  }

}

String APServer::buildPage(){

  Configuration* conf = singleton(Configuration);
  String title = "Settings < Device " + String(conf->Device->number) + " >";
  String page1 = ""
""
"<!DOCTYPE html>"
"<html>"
"<body>"
""
"<h2>" + title + "</h2>"
""
"<script>"
"var configs = new Array();"
"function onLoad() {"
  "var div = document.getElementById(\"customForm\");"
  "var elms = div.getElementsByTagName(\"input\");"
  "console.log(elms);"
  "for(var i = 0, maxI = elms.length; i < maxI; ++i){"
      "var elm = elms[i];"
      "configs[i] = elm.value;"
  "}"
"}"
"window.onload = onLoad;"
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
      "if (elm.value != configs[i]){"
        "data += elm.name + \"=\" + elm.value + \"&\";"
      "}"
  "}"
  "console.log(data);"
  "if (data.length > 0){"
    "xhttp.onreadystatechange = function() {"
      "if (xhttp.readyState == 4 && xhttp.status == 200) {"
        "location.reload();document.getElementById(\"demo\").innerHTML = xhttp.responseText;"
      "}"
    "};"
    "xhttp.open(\"POST\", document.URL + \"save\", true);"
    "xhttp.setRequestHeader(\"Content-type\", \"application/x-www-form-urlencoded\");"
    "xhttp.send(data);"
    "var log = document.getElementById(\"log\");"
    "log.innerHTML = \"Saving...\";"
  "} else {"
    "location.reload();"
  "}"
"}"
"</script>"
"<div id=\"customForm\">";
  yield();
  vector<IStringConvertibleVariable*> vars = conf->toVars();
  yield();
  String inputs = ""
  "<table border=\"1\">"
    "<tr>"
    "<th>Config</th>"
    "<th>Value</th>"
    "</tr>";

  /* styles are commented because of heap memory usage */
  String styleForInput = "";//;"STYLE=\"text-align: center; color: #ffffff; font-weight: bold; background-color: #5ba4a0;\"";
  String styleForPersistentInput = "";//"STYLE=\"text-align: center; color: #ffffff; font-weight: bold; background-color: #407270;\"";

  for(int i = 0; i < vars.size(); i++){
    yield();
    IStringConvertibleVariable* var = vars[i];
    inputs += "<tr>";
    inputs += "<td>" + var->getTag() + "</td>";
    inputs += String("<td align=\"center\">") + "<input align=\"middle\"" + (var->isPersistentVariable() ? styleForPersistentInput : styleForInput) + " type=\"text\" name=\"" + var->getTag() +"\" value=\"" + var->getString() + "\"><br>" + "</td>";
    inputs += "</tr>";
  }

  yield();
  inputs += "<tr> <td align=\"right\" cellpadding=\"5\" id=\"log\"> </td> <td cellpadding=\"5\" align=\"right\"> <button type=\"button\" onclick=\"loadDoc()\">Save</button> </td> </tr>";
  inputs += "</table>";

String page2 = ""
""
"</div>"
""
"</body>"
"</html>"
;

  return page1 + inputs + page2;
}

void APServer::configurationChanged(){
  Serial.println("APServer::configurationChanged");
  setup();
}
