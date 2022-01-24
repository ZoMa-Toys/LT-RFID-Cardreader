#ifndef card_h
#define card_h

//RFID
#include "WiFIAndWeb.h"

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 15      //SDA
#define RST_PIN 16     //  RST - SCL

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
const unsigned long period = 1000;
String lastUID;

#ifndef TRAINNAME
#define TRAINNAME "DUMMY"
#endif
String TrainName=TRAINNAME;
StaticJsonDocument<1024> CardMap;


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


void setupMFRC(){
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  delay(500);
}

String getUID(byte *buffer, byte bufferSize) {
  String UID;
  for (byte i = 0; i < bufferSize; i++) {
    UID.concat(String(buffer[i] < 0x10 ? "0" : ""));
    UID.concat(String(buffer[i], HEX));
  }
  UID.toUpperCase();
  return UID;
}

StaticJsonDocument<200> TrainCard;
void SetTranCardMap(){
  String Maestro="BF189BB5";
  String hetes="E49E2E33";
  TrainCard[Maestro]="Yellow";
  TrainCard[hetes]="Green";
}

void fillCardMap(String UID){
  if(!CardMap.containsKey(UID)){
    int nextIndex = CardMap.size();
    CardMap[UID]=nextIndex;
    debugPrint(UID + " added with index " + nextIndex);
    messageJSONToSend["action"]="CardMap";
    messageJSONToSend["message"]=CardMap;
    sendJSON();
  }
}


void cardLoop() {
  currentMillis=millis();
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  String UID = getUID(mfrc522.uid.uidByte,mfrc522.uid.size);  
  if (TrainCard.containsKey(UID)){
    TrainName = TrainCard[UID].as<String>();
    debugPrint("Train name is: " + TrainName);
  }
  else if (UID!=""){
    fillCardMap(UID);
    if ((currentMillis - startMillis)>period || lastUID != UID){
      messageJSONToSend["action"]="cardChecked";
      JsonObject message = messageJSONToSend.createNestedObject("message");
      int CardIndex = CardMap[UID].as<int>();
      message["train"]=TrainName;
      message["cardIndex"]=CardIndex;
      startMillis=currentMillis;
      lastUID=UID;
    }
  }
} 

#endif 