#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include "WiFiDevice.h"



void offFlagValid(WiFiData _inputData) { // call if need to off valid flag in SSID PASS pair struct
    //Serial.println("NOT VALID");
    _inputData.isValid = false;
    EEPROM.put(0,_inputData);
    EEPROM.commit();
}

void resetWiFiData(WiFiData _inputData){
    Serial.println("reset SSID and PASSWORD");
    offFlagValid(_inputData);
    digitalWrite(LED_BUILTIN, HIGH);   // blink LED
    delay(100);              
    digitalWrite(LED_BUILTIN, LOW);    
}

button::button (byte pin) {
    _pin = pin;
    pinMode(_pin, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
}


        
        

bool button::click(WiFiData _inputData) {
    bool btnState = digitalRead(_pin);
    if (!btnState && !_flag && millis() - _tmr >= 100) {
        _flag = true;
        _tmr = millis();
        return true;
    }
    if (!btnState && _flag && millis() - _tmr >= 3000) {
        _tmr = millis ();
        resetWiFiData(_inputData);
        return true;
    }
    if (btnState && _flag) {
        _flag = false;
        _tmr = millis();
    }
    return false;
}

WiFiDevice::WiFiDevice(const char* ssid_AP = "smart_locker", const char* password_AP = "12345678") : _server(80) {
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

    _server.on("/", handleRoot);
    _server.on("/login", handleLogin);
    _server.on("/inline", []() {
        _server.send(200, "text/plain", "Necessary information about device");
    });

    _server.onNotFound(handleNotFound);
    //here the list of headers to be recorded
    const char * headerkeys[] = {"User-Agent", "Cookie"} ;
    size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
    //ask server to track these headers
    _server.collectHeaders(headerkeys, headerkeyssize);
    _server.begin();
    Serial.println("HTTP server started");
}

bool WiFiDevice::doConnect(WiFiData inputData){
    if(!inputData.isValid){
        Serial.println("Not valid SSID or PASSWORD");
        return false;
    }
    Serial.println("Trying to connect...");
    WiFi.begin(inputData.input_ssid, inputData.input_pass);
    WiFi.setAutoReconnect(true);
    Serial.println("");

    byte msConnection_counter = 0;
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

void WiFiDevice::handleLogin() { //login page, also called for disconnect
    String msg;
    //WebServer server;
    if (_server.hasArg("SSID") && _server.hasArg("PASSWORD")) {
        inputData.input_ssid = _server.arg("SSID");
        inputData.input_pass = _server.arg("PASSWORD");
        inputData.isValid = true;
        EEPROM.put(0, inputData); // got SSID and PASSWORD, now remember
        EEPROM.commit();

        if (WiFiDevice::doConnect(inputData)) {
        _server.sendHeader("Location", "/");
        _server.send(301);
        Serial.println("Log in Successful");
        return;
        }
        msg = "Wrong ssid/password! try again.";
        Serial.println("Log in Failed");
    }
    String content = "<html><body><form action='/login' method='POST'><br>";
    content += "SSID name:<input type='text' name='SSID' placeholder='SSID'><br>";
    content += "Password:<input type='password' name='PASSWORD' placeholder='password'><br>";
    content += "<input type='submit' name='SUBMIT' value='Submit'></form>" + msg + "<br>";
    content += "<a href='/inline'>Info</a></body></html>";
    _server.send(200, "text/html", content);
}



void WiFiDevice::handleRoot() { //root page can be accessed only if authentification is ok
    Serial.println("Enter handleRoot");
    if (!inputData.isValid) {
        _server.sendHeader("Location", "/login");
        _server.send(301);
        return;
    }
    String content = "<html><body><H2>You successfully connected to Wi-Fi!</H2><br>";
    content += "To reset Wi-Fi settings push and hold reset button for 3 sec, then restart device <br>";
    content += "<a href='/inline'>Info</a></body></html>";
    _server.send(200, "text/html", content);
}

void WiFiDevice::handleNotFound() { //no need authentification
    static String message = "Error 404, this page does not exist\n\n";
    message += "URI: ";
    message += WiFiDevice::(_server.uri());
    message += "\nMethod: ";
    message += (_server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += _server.args();
    message += "\n";
    for (uint8_t i = 0; i < _server.args(); i++) {
        message += " " + _server.argName(i) + ": " + _server.arg(i) + "\n";
    }
    WiFiDevice::_server.send(404, "text/plain", message);
}

void WiFiDevice::setup_set() {

}


void WiFiDevice::serverLoop(void) {
    if (millis() - serverTimer >= 100) {   // 10 times at second - server handler
        serverTimer = millis();            
        _server.handleClient();
    }
    if (millis() - buttonTimer >= 500) {   // 2 times at second - reset button handler
        buttonTimer = millis();             
        resetButton.click();
    }
}
    
    








