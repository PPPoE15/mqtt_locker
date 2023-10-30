#include "WiFiDevice.h"
#include <ArduinoJson.h>

WiFiDevice SmartLocker;



void setup(){
  Serial.begin(115200);
  while (!Serial) continue;
  SmartLocker.Init("smart_locker", "12345678");

  
  


}



void loop(){

    
  SmartLocker.serverLoop();
  delay(2);
  

}