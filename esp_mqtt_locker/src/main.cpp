#include "WiFiDevice.h"

WiFiDataStruct setup_EEPROM(void){
    EEPROMClass WiFiState("Device_1");
    WiFiState.begin(512);
    if(!WiFiState.begin(0x500)){
        Serial.println("Failed to initialise WiFiState");
        Serial.println("Restarting...");
        delay(1000);
        ESP.restart();
    }
    WiFiDataStruct _inputData;
    EEPROM.get(0, _inputData);
    return(_inputData);
}

void setup(void){
  Serial.begin(115200);

  delay(500);

  Serial.println("BEGIN");
  delay(500);
  
}

WiFiDevice SmartLocker(setup_EEPROM(), "smart_locker_wifi", "12345678");

void loop(void){

  delay(500);
  Serial.println("LOOP");  
}