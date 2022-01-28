#include "card.h"

void setup() {
  Serial.begin(115200);
  // Connect to wifi
  connectWifi();
  connectWS();
  createWebSerial(recvMsg);
  createOTA();
  setupMFRC();
  messageJSONToSend["action"]="getCardMap";
  SetTranCardMap();
}

void loop() {
  ArduinoOTA.handle();
  // let the websockets client check for incoming messages
  if(client.available()) {
    client.poll();
    if (!messageJSONToSend.isNull()){
      sendJSON();
    }
  }
  else{
    client.connect(websockets_server_host, websockets_server_port, websockets_server_path);
  }
  cardLoop();

}
