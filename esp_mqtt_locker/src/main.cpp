// TODO: 
// 1. error handler


#include <WiFiDevice.h>



#include "MQTT_Proto.h"
#include "API_Proto.h"
#include "Config.h"




struct ProtoState{
  bool api_proto = true;
  bool mqtt_proto = false;
  char mqttServerIP[16];
  uint16_t mqttServerPort = 1883;
}protoState;





//LIIS password   qw8J*883












class button {
    public:
        button (byte pin) {
            _pin = pin;
            pinMode(_pin, INPUT_PULLUP);
            pinMode(LED_BUILTIN, OUTPUT);
        }
        
        bool click() {
            bool btnState = digitalRead(_pin);
            if (!btnState && !_flag && millis() - _tmr >= 100) {
                _flag = true;
                _tmr = millis();
                return true;
            }
            if (!btnState && _flag && millis() - _tmr >= 3000) {
                _tmr = millis ();
                emergencyOpen();
                return true;
            }
            if (btnState && _flag) {
                _flag = false;
                _tmr = millis();
            }
            return false;
        }

    private:
        byte _pin;
        uint32_t _tmr;
        bool _flag;
        void emergencyOpen(){
          for (int i=0; i < NUM_PLATES; i++){
            Message event2("open", i);
          }
        }
}emgButton(EMG_PIN);









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

