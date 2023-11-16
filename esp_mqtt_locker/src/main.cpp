// TODO: 
// 1. error handler

#include "Worker.h"


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
  // server and API loop
  uint32_t traceTmr = millis();
  smartLocker.serverLoop(); 
  

  if (WiFi.status() != WL_CONNECTED) {
    smartLocker.reconnectWiFi();
    Serial.println("Reconnect to Wi-Fi");
  }

  // MQTT loop
  if(protoState.mqtt_proto){ 
    if(mqttClient.connected()){
      mqttClient.loop();
    }
    else{
      reconnect();
    }
  }


  

  // Emergency button handler
  static uint32_t emgButtonTmr;
  if (millis() - emgButtonTmr >= 300){ 
    emgButtonTmr = millis(); 
    emgButton.click();
  }

  traceTmr = millis() - traceTmr;
  if(traceTmr > 5){
    Serial.println(traceTmr);
  }

}

/*
// RTC loop
  static uint32_t tmrgetTime;
  if (millis() - tmrgetTime >= 60000){
    tmrgetTime = millis(); 
    String time = timeClient.getFormattedTime();
    //Serial.println("Time: " + time);
  }

// UART cleaning
  static uint32_t flushTmr;
  if (millis() - flushTmr >= 300){ // clear uart buffer
    flushTmr = millis(); 
    while(Serial1.available()){
      Serial1.read();
    }
  }
  
*/