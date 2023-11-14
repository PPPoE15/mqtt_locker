#ifndef WORKER_H
#define WORKER_h

#include "MQTT_Proto.h"
#include "API_Proto.h"
#include "Config.h"


struct ProtoState{
  bool api_proto = true;
  bool mqtt_proto = false;
  char mqttServerIP[16];
  uint16_t mqttServerPort = 1883;
}protoState;

void settingsHandler(){
  if(smartLocker.server.hasArg("api_proto")){
    Serial.println(smartLocker.server.arg("api_proto"));
    protoState.api_proto = true;  
  } 
  else{
    protoState.api_proto = false;
  }
  if(smartLocker.server.hasArg("mqtt_proto")){
    Serial.println(smartLocker.server.arg("mqtt_proto"));
    protoState.mqtt_proto = true;
    if(smartLocker.server.hasArg("mqttIP") & smartLocker.server.arg("mqttIP") != ""){
      String str = smartLocker.server.arg("mqttIP");
      str.toCharArray(protoState.mqttServerIP, 16);
    }
    if(smartLocker.server.hasArg("mqttPort") & smartLocker.server.arg("mqttPort") != ""){
      protoState.mqttServerPort = smartLocker.server.arg("mqttPort").toInt();
    }
    initMQTT(protoState.mqttServerIP, protoState.mqttServerPort);
  }
  else{
    protoState.mqtt_proto = false;
  }
  EEPROM.put(PROTO_STATE_ADDR, protoState);
  EEPROM.commit();
  
  Serial.println(protoState.api_proto);
  Serial.println(protoState.mqtt_proto);
  String apiChecked = "checked";
  if(!protoState.api_proto){
    apiChecked = "";
  }
  String mqttChecked = "";
  if(protoState.mqtt_proto){
    mqttChecked = "checked";
  }


  String content = "<html><body><form action='/settings' method='POST'><br>";
  content += "<br>API enabled by default<br>";
  content += "<input type='checkbox' id='api' name='api_proto' value='api'" + apiChecked + "/> <label for='api'>api</label> <br>";
  content += "<input type='checkbox' id='mqtt' name='mqtt_proto' value='mqtt'" + mqttChecked + "/> <label for='mqtt'>mqtt</label> <br>";
  content += "MQTT broker IP:<input type='text' name='mqttIP' placeholder='0.0.0.0'><br>";
  content += "MQTT broker port:<input type='text' name='mqttPort' placeholder='1883'><br>";
  content += "<input type='submit' name='SUBMIT' value='Submit'></form> <br>";
  content += "<a href='/info'>Info</a></body></html>";
  smartLocker.server.send(200, "text/html", content);
    
}


void setup_routing() { 
  smartLocker.addHandler("/api/Login", HTTP_POST, logined);
  smartLocker.addHandler("/api/DeviceTasks", HTTP_PUT, deviceTasks);
  smartLocker.addHandler("/settings", settingsHandler);
  smartLocker.addHandler("/api/GetVersion", HTTP_GET, getVersion);
} 

#endif // WORKER_H