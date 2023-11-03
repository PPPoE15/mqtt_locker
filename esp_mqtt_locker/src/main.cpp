// TODO: 
// 1. create a message class
// 2. error handler
// 3. implement checking all locker status

#include <WiFiDevice.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <Random16.h>


#define RXD2 18 // uart1 pins for locker control
#define TXD2 19

#define statusHeader 0x80
#define   openHeader 0x8A
#define   openCode   0x11
#define statusCode   0x33

#define numLockersOnPlate 50
#define numPlates 1
#define NUM_BYTES 7



//LIIS password   qw8J*883

WiFiDevice smartLocker;

StaticJsonDocument<250> jsonDocument;
char buffer[250];
byte controlMessage[] = {0x8A, 0x01, 0x18, 0x11, 0x9B};
byte feedbackMessage[5];
//byte allLockerstatus[11];
byte check_sum;

WiFiUDP ntpUDP; 
NTPClient timeClient(ntpUDP,"0.pool.ntp.org", 10800, 60000);

String bearer;

Random16 rnd; //Более легкий рандом чем оригинальная библиотека




byte* create_message(byte header, byte plate_addr, byte lock_addr, byte function_code){
  controlMessage[0] = header;
  controlMessage[1] = plate_addr;
  controlMessage[2] = lock_addr;
  controlMessage[3] = function_code;
  controlMessage[4] = controlMessage[0] ^ controlMessage[1] ^ controlMessage[2] ^ controlMessage[3];
  return controlMessage;
}

void checkAllLockers(byte device_id){
  jsonDocument.clear();
  JsonArray opened = jsonDocument.createNestedArray("closed");

  uint8_t allLockerstatus[11];
  while(Serial1.available()){
    Serial1.read();
  }
  delay(50);
  Serial1.write(create_message(statusHeader, device_id, 0, statusCode), 5);
  delay(50);
  Serial1.read(allLockerstatus, 11);
  delay(50);

  for (int i=2; i<9; i++){  // for each of the 7 byte in feedback message

    uint8_t status = allLockerstatus[i];
    Serial.println( status );
    for (int j=7; j>=0; j--){  // for each of the 8 lockers in one byte
      if(status != status % (1 << j)){
        
        status %= (1 << j); 
        opened.add( (NUM_BYTES + 1 - i)*8 + j+1 );  // calculates num of the locker on the board
        Serial.println( (NUM_BYTES + 1 - i)*8 + j+1 ); 
        Serial.println("");
      }
    }
  }
  serializeJson(jsonDocument, buffer);
  smartLocker.server.send(200, "application/json", buffer);
  while(Serial1.available()){
    Serial1.read();
  }
}

String requestHandler(byte device_id, byte event_id, byte value){
  switch (event_id)
  {
    case 1: // event: check the STATUS of ALL lockers 
      checkAllLockers(device_id);
      return("");
    break;

    case 2: // event: OPEN ALL lockers
      Serial1.write(create_message(openHeader, device_id, 0, openCode), 5);
      return("open_all");
    break;

    case 3: // event: check the STATUS of ONE locker
      Serial1.write(create_message(statusHeader, device_id, value, statusCode), 5);
      Serial1.read(feedbackMessage, 5);
      if(feedbackMessage[3] == 0x11){  // check if successful unlocking 
        return "closed";
      }
      else{
        return "open";
      }
    break;

    case 4: // event: OPEN ONE locker 
      Serial1.write(create_message(openHeader, device_id, value, openCode), 5);
      Serial1.read(feedbackMessage, 5);
        while(Serial1.available()){
          Serial1.read();
        }
        if(feedbackMessage[3] == 0x11){  // check if successful unlocking 
          return "fail";
        }
        else{
          return "success";
        }
    break;

    default:
      Serial.println("Unsupported event_id");
      return("Unsupported event_id");
    break;
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



void setup_routing() { 
  smartLocker.addHandler("/api/Login", HTTP_POST, logined);
  smartLocker.addHandler("/api/DeviceTasks", HTTP_PUT, deviceTasks);
}



void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);

  //debug
  /*
  byte allFeedbackMessage [11];
  Serial1.write(create_message(statusHeader,0, 0, statusCode), 5);
  Serial1.read(allLockerstatus, 11);  
  Serial.println(isLockersOpen(allLockerstatus[8]));
  */
  
  smartLocker.Init("smart_locker", "12345678"); // AP settings
  setup_routing(); 
  timeClient.begin();
  timeClient.update();
  bearer = generateRandomString(60); // initial generating berear token
  Serial.println(smartLocker.getIP());
  
}



void loop() {

  smartLocker.serverLoop();

  static uint32_t tmrgetTime;
    if (millis() - tmrgetTime >= 60000)
   {
      tmrgetTime = millis(); 
      String time = timeClient.getFormattedTime();
      //Serial.println("Time: " + time);
   }

  static uint32_t flushTmr;
  if (millis() - flushTmr >= 300) // clear uart buffer
  {
    flushTmr = millis(); 
    while(Serial1.available()){
      Serial1.read();
    }

  }

}

