// TODO: 
// 1. error handler






#include "Worker.h"










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

