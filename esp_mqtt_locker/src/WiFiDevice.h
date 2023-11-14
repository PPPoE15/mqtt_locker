/*
    WiFiDevice.h - realize functionality of a Wi-Fi device on ESP32 board. 
    Support only one simultaneous WiFiDevice class object.
    Use the library to create a device with the following functionality:
        1. After the first startup or reset the device creates access point.
        2. You should to set SSID name, password and other settings by connecting to the AP via 192.168.4.1. 
        3. Then the device will save settings in the EEPROM and connect to an existing Wi-Fi network.
        4. Even after a reboot the device will connect to Wi-Fi. 
        5. To reset the settings pull resetWiFiButton pin to the ground for 3 seconds until the LED_BUILTIN led flashes, then restart the device.
    How to use the ibrary:
        1. Create a WiFiDevice class object.
        2. Call the Init method and set main settings.
        3. 
*/

#pragma once

#include <WebServer.h>
#include <EEPROM.h>


#ifndef LED_BUILTIN
  #define LED_BUILTIN 4
#endif


#ifndef resetWiFiPin
    #define resetWiFiPin 21
#endif


struct WiFiDataStruct {
    String input_ssid;
    String input_pass;
    bool isValid;
};


class WiFiDevice{
    public:
        WiFiDataStruct inputData;
        WebServer server;
        WiFiDevice();
        void Init(const char* ssid_AP, const char* password_AP);
        void serverLoop();
        void addHandler(Uri uri, WebServer::THandlerFunction Handler);
        void addHandler(Uri uri, http_method method, WebServer::THandlerFunction Handler);
        IPAddress getIP();
        void reconnectWiFi();

    private:
        static void offFlagValid(WiFiDataStruct _inputData);
        bool doConnect(WiFiDataStruct _inputData);
        void handleLogin();
        void handleRoot();
        void handleNotFound();
        static void resetWiFiData(WiFiDataStruct _inputData);
        IPAddress serverIP;

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
        } resetButton{resetWiFiPin};

};



