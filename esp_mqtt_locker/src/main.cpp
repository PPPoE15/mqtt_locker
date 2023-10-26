#include "WiFiDevice.h"
//#include <EEPROM.h>



void setup(void){
  Serial.begin(115200);
  


  delay(500);

  Serial.println("BEGIN");

  delay(500);

  
  
}



void loop(void){
  WiFiDevice SmartLocker("smart_locker_wifi", "12345678");
  Serial.println("LOOP");  
  while(true){
      SmartLocker.serverLoop();
      delay(2);
  }
  delay(500);
  
  

}