#include "WiFIAndWeb.h"





void setup() {
  Serial.begin(115200);
  // Connect to wifi
  connectWifi();
  connectWS();
  createWebSerial();
  createOTA();
  setupMFRC();
}

void loop() {
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