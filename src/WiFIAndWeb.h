#ifndef WiFIAndWeb_h
#define WiFIAndWeb_h

#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>
#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>

#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#ifndef STASSID
#define STASSID "SOMEWIFI"
#endif
#ifndef STAPSK
#define STAPSK  "SecretPW"
#endif

#ifndef WSHOST
#define WSHOST "RANDOMHOST"
#endif

#ifndef WSPORT
#define WSPORT 80
#endif


const char* ssid = STASSID;
const char* password = STAPSK;

using namespace websockets;

const char* websockets_server_host = WSHOST; //Enter server adress
const uint16_t websockets_server_port = WSPORT; // Enter server port
const char* websockets_server_path = "/ws"; //Enter server adress
String debug = "";
StaticJsonDocument<2048>  messageJSONToSend;

String TrainName="a";
StaticJsonDocument<1024> CardMap;


WebsocketsClient client;
AsyncWebServer server(80);

void debugPrint(String toprint){
    if (debug == "Serial"){
        Serial.println(toprint);
    }
    else if (debug == "webserial"){
        WebSerial.println(toprint);
    }
    else if (debug == "websocket"){
        StaticJsonDocument<2048> tp;
        tp["DEBUG"]=toprint;
        String message;
        serializeJson(tp,message);
        client.send(message);
    }
}

void sendJSON(){
    String toSend = "";
    serializeJson(messageJSONToSend,toSend);
    messageJSONToSend.clear();
    debugPrint("toSend: " + toSend);
    client.send(toSend);
    toSend = "";

}

#include "card.h"


void onDataReceived(String msg){
  debugPrint("Incoming WS msg: " + msg);
  StaticJsonDocument<40096>  messageJSON;
  if (msg.indexOf("Status")>-1 && msg.indexOf("CardMap")>-1){
      DeserializationError error = deserializeJson(messageJSON, msg);
      if (error) {
          debugPrint("deserializeJson() failed: ");
          debugPrint(error.f_str());
      return;
      }
  }
  if(messageJSON.containsKey("Status")){
      if(messageJSON["Status"]=="CardMap:"){
          debugPrint("Fill Cardmap");
          CardMap = messageJSON["Message"];
      }
  }
}


void connectWifi(){
    WiFi.begin(ssid, password);

    // Wait some time to connect to wifi
    Serial.println("connecting to WiFi");
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println("Connection Failed! Rebooting...");
      delay(5000);
      ESP.restart();
    }

    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void connectWS(){
    Serial.println("Connecting to server.");
    // try to connect to Websockets server
    bool connected = client.connect(websockets_server_host, websockets_server_port, websockets_server_path);
    if(connected) {
        Serial.println("Connected!");
        client.send("Hello Server");
    } else {
        Serial.println("Not Connected!");
    }
    client.onMessage([&](WebsocketsMessage message){
        String msg;
        msg=message.data();
        onDataReceived(msg);
    });
}

void recvMsg(uint8_t *data, size_t len){
  WebSerial.println("Received Data...");
  String d = "";
  for(int i=0; i < len; i++){
    d += char(data[i]);
  }
  if (d.indexOf(String("DebugOff"))>-1){
    WebSerial.println("Debug Deactivated");
    debug = "";
  }
  else if (d.indexOf(String("DebugOn"))>-1){
    WebSerial.println("Debug Activated");
    debug = "webserial";
  }
  else if (d.indexOf(String("ResetESP"))>-1){
    WebSerial.println("Reseting ESP");
    ESP.restart();
  }
  else if (d.indexOf(String("NewTrainName:"))>-1){
    TrainName = d.substring(13,d.length());
    WebSerial.print("Setting TrainName to ");
    WebSerial.println(TrainName);
  }
  else if (d.indexOf(String("ShowCards"))>-1){
    String dp;
    serializeJson(CardMap,dp);
    WebSerial.println("CardMap:" + dp);
  }
  else if (d.indexOf(String("Help"))>-1){
    WebSerial.println("NewTrainName:<NAME OF THE TRAIN>");
    WebSerial.println("DebugOff");
    WebSerial.println("DebugOn");
    WebSerial.println("ResetESP");
    WebSerial.println("ShowCards");
    WebSerial.println("asdfadhsd");
  }
  WebSerial.println(d);
}

void createWebSerial(){
  WebSerial.begin(&server);
    /* Attach Message Callback */
  WebSerial.msgCallback(recvMsg);
  server.begin();
}

void createOTA(){
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}




#endif 