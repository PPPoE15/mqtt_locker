// TODO: 
// 1. error handler


#include <WiFiDevice.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <Random16.h>

#include "MQTT_Proto.h"



#define RXD2 18 // uart1 pins for locker control
#define TXD2 19

#define statusHeader 0x80
#define   openHeader 0x8A
#define   openCode   0x11
#define statusCode   0x33

#define numLockersOnPlate 50
#define NUM_PLATES 1
#define NUM_BYTES 7

#define EMG_PIN 5
#define PROTO_STATE_ADDR 100
struct ProtoState{
  bool api_proto = true;
  bool mqtt_proto = false;
  char mqttServerIP[16];
  uint16_t mqttServerPort = 1883;
}protoState;





//LIIS password   qw8J*883

WiFiDevice smartLocker;



StaticJsonDocument<250> jsonDocument;
char buffer[250];
WiFiUDP ntpUDP; 
NTPClient timeClient(ntpUDP,"0.pool.ntp.org", 10800, 60000);
String bearer;
Random16 rnd; //Более легкий рандом чем оригинальная библиотека





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


void checkAllLockers(byte device_id){
  jsonDocument.clear();
  JsonArray opened = jsonDocument.createNestedArray("closed");
  uint8_t allLockerstatus[11];
  Message message("check", device_id, allLockerstatus);

  for (int i=2; i<9; i++){  // for each of the 7 byte in feedback message
    uint8_t status = allLockerstatus[i];
    for (int j=7; j>=0; j--){  // for each of the 8 lockers in one byte
      if(status != status % (1 << j)){
        
        status %= (1 << j); 
        opened.add( (NUM_BYTES + 1 - i)*8 + j+1 );  // calculates num of the locker on the board
      }
    }
  }
  serializeJson(jsonDocument, buffer);
  smartLocker.server.send(200, "application/json", buffer);
}

String requestHandler(byte device_id, byte event_id, byte value){
  byte feedbackMessage[5];

  switch (event_id)
  {
    case 1: { // event: check the STATUS of ALL lockers 
      checkAllLockers(device_id);
      return("");
    } break;

    case 2: { // event: OPEN ALL lockers
      Message event2("open", device_id);
      return("open_all");
    } break;

    case 3: { // event: check the STATUS of ONE locker
      Message event3("check", device_id, value, feedbackMessage);
      if(feedbackMessage[3] == 0x11){  // check if successful unlocking 
        return "closed";
      }
      else{
        return "open";
      }
    } break;

    case 4: { // event: OPEN ONE locker 
      Message event4("open", device_id, value, feedbackMessage);
      if(feedbackMessage[3] == 0x11){  // check if successful unlocking 
        return "fail";
      }
      else{
        return "success";
      }
  } break;

    default: {
      Serial.println("Unsupported event_id");
      return("Unsupported event_id");
    } break;
  }
}


String generateRandomString(int length) { //функция генерации случайной строки

  rnd.setSeed(timeClient.getEpochTime());
  String characters = "abcdefghijklmnopqrstuvwxyz1234567890"; //строка из которой берутся символы для генерации
  String randomString = "";

  for (int i = 0; i < length; i++) {
    int randomIndex = rnd.get(characters.length());
    randomString += characters[randomIndex];
  }

  return randomString;
}



void create_json_bearer(String status, String name) { //функция для сборки JSON ответа с ключом
  jsonDocument.clear();
  JsonObject root = jsonDocument.createNestedObject();
  root["status"] = status;
  JsonObject session = root.createNestedObject("session");
  bearer = generateRandomString(60);
  session["name"] = name;
  session["value"] = bearer;
  serializeJson(root, buffer); 
}



void logined() { //функция авторизации
  Serial.println("Lohined handler");
  if (smartLocker.server.hasArg("plain") == false) {
    //обработка ошибки TODO
  }

  String body = smartLocker.server.arg("plain");
  deserializeJson(jsonDocument, body);
  String login = jsonDocument["login"];
  String password = jsonDocument["password"];

  if ((login == "admin")&&(password == "123456")){
    create_json_bearer("success", login);  
    smartLocker.server.send(200, "application/json", buffer);
  }
  else{
    jsonDocument.clear();  
    jsonDocument["status"] = "false";
    serializeJson(jsonDocument, buffer);
    smartLocker.server.send(200, "application/json", buffer);
  }
}



void create_json_response(String status, int task_id) { //функция ответа на запрос DeviceTasks
  jsonDocument.clear();
  jsonDocument["status"] = status;
  jsonDocument["task_id"] = task_id;
  serializeJson(jsonDocument, buffer); 
}

void getVersion(){
  jsonDocument.clear();
  jsonDocument["version"] = "1.8";
  serializeJson(jsonDocument, buffer);
  smartLocker.server.send(200, "application/json", buffer);
}

void deviceTasks(){
  if (smartLocker.server.hasArg("plain") == false) {
    //обработка ошибки TODO
  }
  String token;
  if (smartLocker.server.hasHeader("Authorization")) {
    String authHeader = smartLocker.server.header("Authorization");
    Serial.println("Authorization Header: " + authHeader);
    
    // Разбор значения токена
    if (authHeader.startsWith("Bearer ")) {
      token = authHeader.substring(7); // Извлекаем часть после "Bearer "
      Serial.println("Bearer Token: " + token);
    }
  }
  if (token == bearer){
    String body = smartLocker.server.arg("plain");
    deserializeJson(jsonDocument, body);
    byte device_id = jsonDocument["device_id"]; // plate number
    byte event_id = jsonDocument["event_id"]; 
    byte value = jsonDocument["value"]; // locker number 
    String feedback = requestHandler(device_id, event_id, value); // works with response
    create_json_response(feedback, event_id);
    smartLocker.server.send(200, "application/json", buffer);
  }
  else{
    jsonDocument.clear();
    jsonDocument["error"] = "unauthorized";
    serializeJson(jsonDocument, buffer);
    smartLocker.server.send(401, "application/json", buffer);
  } 
}






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

