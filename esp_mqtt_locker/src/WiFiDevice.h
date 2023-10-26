#pragma once

#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>


#ifndef LED_BUILTIN
  #define LED_BUILTIN 4
#endif

#define resetWiFiButton 21


struct WiFiDataStruct {
    String input_ssid;
    String input_pass;
    bool isValid;
};


class WiFiDevice{
    public:
        WiFiDataStruct inputData;
        WebServer _server;
        WiFiDevice();
        void Init(char* ssid_AP, char* password_AP);
        void serverLoop(void);

    private:
        static void offFlagValid(WiFiDataStruct _inputData);
        bool doConnect(WiFiDataStruct _inputData);
        void handleLogin();
        void handleRoot();
        void handleNotFound();
        static void resetWiFiData(WiFiDataStruct _inputData);

        class button {
            public:
                button (byte pin) {
                    _pin = pin;
                    pinMode(_pin, INPUT_PULLUP);
                    pinMode(LED_BUILTIN, OUTPUT);
                }
                
                bool click(WiFiDataStruct inputData) {
                    bool btnState = digitalRead(_pin);
                    if (!btnState && !_flag && millis() - _tmr >= 100) {
                        _flag = true;
                        _tmr = millis();
                        return true;
                    }
                    if (!btnState && _flag && millis() - _tmr >= 3000) {
                        _tmr = millis ();
                        resetWiFiData(inputData);
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
        } resetButton{resetWiFiButton};

};



