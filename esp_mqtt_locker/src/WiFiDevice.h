#pragma once

#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>

#ifndef LED_BUILTIN
  #define LED_BUILTIN 4
#endif

#define resetWiFiButton 21

struct WiFiData {
    String input_ssid;
    String input_pass;
    bool isValid;
};

void resetWiFiData(WiFiData inputData);

void offFlagValid(WiFiData _inputData) { // call if need to off valid flag in SSID PASS pair struct
    //Serial.println("NOT VALID");
    _inputData.isValid = false;
    EEPROM.put(0,_inputData);
    EEPROM.commit();
}

class button {
    public:
        button (byte pin);
        bool click(WiFiData _inputData);

    private:
        byte _pin;
        uint32_t _tmr;
        bool _flag;
};

class WiFiDevice{
    

    public:
        WiFiData inputData;
        WebServer _server;

        button resetButton{resetWiFiButton};


        void offFlagValid();

        const char* ssid_AP     = "smart_locker";  //office ssid: LIIS
        const char* password_AP = "12345678";  //office password: qw8J*883

        uint32_t serverTimer, buttonTimer;



        bool doConnect(WiFiData inputData);

        void handleLogin();

        void handleRoot();

        void handleNotFound();

        void setup_set();

        void serverLoop(void);
    
        WiFiDevice(const char* ssid_AP = "smart_locker", const char* password_AP = "12345678");



};



