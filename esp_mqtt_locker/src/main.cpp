#ifdef ESP32
  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <WebServer.h>
  //#include <WiFiAP.h>
  #include <EEPROM.h>
  WebServer server(80);
#else
  #include <ESP8266WiFi.h>
  #include <WiFiClient.h>
  #include <ESP8266WebServer.h>
  ESP8266WebServer server(80);
#endif

#ifndef LED_BUILTIN
  #define LED_BUILTIN 4
#endif

#define resetWiFiButton 21

struct WiFiData {
  String input_ssid = "";
  String input_pass = "";
  bool isValid;
};


void offFlagValid();
bool doConnect(WiFiData inputData);
void handleLogin();
void handleRoot();
void handleNotFound();


class button {
  public:
    button (byte pin) {
      _pin = pin;
      pinMode(_pin, INPUT_PULLUP);
      pinMode(LED_BUILTIN, OUTPUT);
    }
    void resetWiFiData(){
      Serial.println("reset SSID and PASSWORD");
      offFlagValid();
      digitalWrite(LED_BUILTIN, HIGH);   // blink LED
      delay(100);              
      digitalWrite(LED_BUILTIN, LOW);    
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
        resetWiFiData();
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
};


WiFiData inputData;
button resetButton(resetWiFiButton);
const char* ssid_AP     = "smart_locker";
const char* password_AP = "12345678";
//const char* ssid     = "LIIS";
//const char* password = "qw8J*883";
uint32_t serverTimer, buttonTimer, resetTimer = 0;


void offFlagValid() { // call if need to off valid flag in SSID PASS pair struct
  Serial.println("NOT VALID");
  inputData.isValid = false;
  EEPROM.put(0,inputData);
  EEPROM.commit();
}


bool doConnect(WiFiData inputData){
  if(!inputData.isValid){
    Serial.println("Not valid SSID or PASSWORD");
    return false;
  }
  Serial.println("Trying to connect...");
  WiFi.begin(inputData.input_ssid, inputData.input_pass);
  WiFi.setAutoReconnect(true);
  Serial.println("");

  int msConnection_counter = 0;
  while (WiFi.status() != WL_CONNECTED) {  // Wait connection for 30 sec
    delay(500);
    Serial.print(".");
    msConnection_counter++;
    if (msConnection_counter > 60){
      Serial.println("ERROR while connecting");
      offFlagValid();
      return false;
    }
  }
  Serial.println("\n");
  Serial.print("Connected to ");
  Serial.println(inputData.input_ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}


void handleLogin() { //login page, also called for disconnect
  String msg;
  if (server.hasArg("DISCONNECT")) {
    Serial.println("Disconnection");
    server.sendHeader("Location", "/login");
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Set-Cookie", "ESPSESSIONID=0");
    server.send(301);
    return;
  }
  if (server.hasArg("SSID") && server.hasArg("PASSWORD")) {
    inputData.input_ssid = server.arg("SSID");
    inputData.input_pass = server.arg("PASSWORD");
    inputData.isValid = true;
    EEPROM.put(0, inputData); // got SSID and PASSWORD, now remember
    EEPROM.commit();

    if (doConnect(inputData)) {
      server.sendHeader("Location", "/");
      server.sendHeader("Cache-Control", "no-cache");
      server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
      server.send(301);
      Serial.println("Log in Successful");
      return;
    }
    msg = "Wrong ssid/password! try again.";
    Serial.println("Log in Failed");
  }
  String content = "<html><body><form action='/login' method='POST'>To log in, please use : admin/admin<br>";
  content += "SSID name:<input type='text' name='SSID' placeholder='SSID'><br>";
  content += "Password:<input type='password' name='PASSWORD' placeholder='password'><br>";
  content += "<input type='submit' name='SUBMIT' value='Submit'></form>" + msg + "<br>";
  content += "<a href='/inline'>Info</a></body></html>";
  server.send(200, "text/html", content);
}


void handleRoot() { //root page can be accessed only if authentification is ok
  Serial.println("Enter handleRoot");
  String header;
  if (!inputData.isValid) {
    server.sendHeader("Location", "/login");
    server.sendHeader("Cache-Control", "no-cache");
    server.send(301);
    return;
  }
  String content = "<html><body><H2>You successfully connected to Wi-Fi!</H2><br>";
  content += "To reset Wi-Fi settings push and hold reset button for 3 sec, then restart device <br>";
  content += "<a href='/inline'>Info</a></body></html>";
  server.send(200, "text/html", content);
}


void handleNotFound() { //no need authentification
  String message = "Error 404, this page does not exist\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void) {
  Serial.begin(115200);
  while(!Serial) {}
  EEPROM.begin(512);
  EEPROM.get(0, inputData);
  Serial.println("Current SSID:");
  Serial.println(inputData.input_ssid);
  Serial.println("Current PASS:");
  Serial.println(inputData.input_pass);

  if (!doConnect(inputData)){ // trying to connect to wi-fi
    Serial.println("\n\nConfiguring access point..."); // if canceled open wi-fi acces point
    if (!WiFi.softAP(ssid_AP, password_AP)) {
      log_e("Soft AP creation failed.");
      while(1);
    }
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
  }

  server.on("/", handleRoot);
  server.on("/login", handleLogin);
  server.on("/inline", []() {
    server.send(200, "text/plain", "Necessary information about device");
  });

  server.onNotFound(handleNotFound);
  //here the list of headers to be recorded
  const char * headerkeys[] = {"User-Agent", "Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
  //ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  if (millis() - serverTimer >= 100) {   // 10 times at second - server handler
    serverTimer = millis();            
    server.handleClient();
  }
  if (millis() - buttonTimer >= 500) {   // 2 times at second - reset button handler
    buttonTimer = millis();             
    resetButton.click();
  }
}