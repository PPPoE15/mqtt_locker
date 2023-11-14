// TODO: 
// 1. error handler

#include "Worker.h"

//LIIS password   qw8J*883

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  smartLocker.Init("smart_locker", "12345678"); // AP settings
  EEPROM.get(PROTO_STATE_ADDR, protoState);
  if(protoState.mqtt_proto){
    initMQTT(protoState.mqttServerIP, protoState.mqttServerPort);
  }
  setup_routing(); 
  timeClient.begin();
  timeClient.update();
  bearer = generateRandomString(60); // initial generating berear token 
}


void loop() {
  smartLocker.serverLoop();
  if(protoState.mqtt_proto){
    if(mqttClient.connected()){
      mqttClient.loop();
    }
    else{
      reconnect();
    }
  }

  static uint32_t tmrgetTime;
  if (millis() - tmrgetTime >= 60000){
    tmrgetTime = millis(); 
    String time = timeClient.getFormattedTime();
    //Serial.println("Time: " + time);
  }

  static uint32_t flushTmr;
  if (millis() - flushTmr >= 300){ // clear uart buffer
    flushTmr = millis(); 
    while(Serial1.available()){
      Serial1.read();
    }

  }

  static uint32_t emgButtonTmr;
  if (millis() - emgButtonTmr >= 300){ // clear uart buffer
    emgButtonTmr = millis(); 
    emgButton.click();
  }

}

