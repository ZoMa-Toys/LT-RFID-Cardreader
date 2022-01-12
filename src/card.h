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

String dump_byte_array(byte *buffer, byte bufferSize) {
  String UID;
  for (byte i = 0; i < bufferSize; i++) {
    UID.concat(String(buffer[i] < 0x10 ? " 0" : " "));
    UID.concat(String(buffer[i], HEX));
  }
  UID.toUpperCase();
  return UID.substring(1);
}


void fillCardMap(String UID){
  if(!CardMap.containsKey(UID)){
    int nextIndex = CardMap.size();
    CardMap[UID]=nextIndex;
    debugPrint(UID + " added with index " + nextIndex);
    messageJSONToSend["action"]="CardMap";
    StaticJsonDocument<1024> message = messageJSONToSend.createNestedObject("message");
    message = CardMap;
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
  debugPrint("UID tag :");  
  String UID= dump_byte_array(mfrc522.uid.uidByte,mfrc522.uid.size);
  fillCardMap(UID);
  if (UID!=""){
    messageJSONToSend["action"]="cardChecked";
    StaticJsonDocument<200> message = messageJSONToSend.createNestedObject("message");
    message["train"]=TrainName;
    message["cardIndex"]=CardMap[UID];
  }
} 

#endif 