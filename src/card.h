#ifndef card_h
#define card_h

//RFID
#include "WiFIAndWeb.h"

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 15      //SDA
#define RST_PIN 16     //  RST - SCL

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.


void setupMFRC(){
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  delay(500);
}

String getUID(byte *buffer, byte bufferSize) {
  String UID;
  for (byte i = 0; i < bufferSize; i++) {
    UID.concat(String(buffer[i] < 0x10 ? " 0" : " "));
    UID.concat(String(buffer[i], HEX));
  }
  UID.toUpperCase();
  debugPrint("UID UC:" + UID);
  UID.substring(1);
  debugPrint("UID SS:" + UID.substring(1));
  UID = UID.substring(1);
  return UID;
}


void fillCardMap(String UID){
  if(!CardMap.containsKey(UID)){
    int nextIndex = CardMap.size();
    CardMap[UID]=nextIndex;
    debugPrint(UID + " added with index " + nextIndex);
    messageJSONToSend["action"]="CardMap";
    messageJSONToSend["message"]=CardMap;
 //   JsonObject message = messageJSONToSend.createNestedObject("message");
  //  message = CardMap.as<JsonObject>();
    String dp;
    serializeJson(messageJSONToSend,dp);
    debugPrint("UMJ" + dp);
  }
}


void cardLoop() {
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
  debugPrint("UID tag :" + UID);  
  delay(1000);
  fillCardMap(UID);
  if (UID!=""){
    messageJSONToSend["action"]="cardChecked";
    JsonObject message = messageJSONToSend.createNestedObject("message");
    int CardIndex = CardMap[UID].as<int>();

    message["train"]=TrainName;
    message["cardIndex"]=CardIndex;
  }
} 

#endif 