#ifndef WiFIAndWeb_h
#define WiFIAndWeb_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <WebSerial.h>
#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>

using namespace websockets;

const char* ssid = "Guber-Kray"; //Enter SSID
const char* password = "Hafnium1985!"; //Enter Password
const char* websockets_server_host = "192.168.1.88"; //Enter server adress
const uint16_t websockets_server_port = 80; // Enter server port
const char* websockets_server_path = "/ws"; //Enter server adress
String debug = "";
StaticJsonDocument<2048>  messageJSONToSend;

String TrainName="Green";
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
  StaticJsonDocument<2048>  messageJSON;
  if (msg.indexOf("TrackConfig")==-1){
      DeserializationError error = deserializeJson(messageJSON, msg);
      if (error) {
          debugPrint("deserializeJson() failed: ");
          debugPrint(error.f_str());
      return;
      }
  }

  if(messageJSON.containsKey("action")){
    if(messageJSON["action"]=="CardMap"){
          debugPrint("Fill Cardmap");
    }
  }
    
  else if(messageJSON.containsKey("Status")){
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
    for(int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
        Serial.print(".");
        delay(1000);
    }

    // Check if connected to wifi
    if(WiFi.status() != WL_CONNECTED) {
        Serial.println("No Wifi!");
        return;
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
  else if (d.indexOf(String("TrainName:"))>-1){
    TrainName = d.substring(11,d.length()).toInt();
    WebSerial.print("Setting TrainName to ");
    WebSerial.println(TrainName);
  }
  else if (d.indexOf(String("Help"))>-1){
    WebSerial.println("TrainName:<NAME OF THE TRAIN>");
    WebSerial.println("DebugOff");
    WebSerial.println("DebugOn");
    WebSerial.println("ResetESP");
  }
  WebSerial.println(d);
}

void createWebSerial(){
  WebSerial.begin(&server);
    /* Attach Message Callback */
  WebSerial.msgCallback(recvMsg);
}

void createOTA(){
  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
}




#endif 