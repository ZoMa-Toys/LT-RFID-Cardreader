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