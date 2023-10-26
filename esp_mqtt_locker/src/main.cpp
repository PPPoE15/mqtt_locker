#include "WiFiDevice.h"

WiFiDevice SmartLocker;

void setup(void){
  Serial.begin(115200);
  SmartLocker.Init("smart_locker", "12345678");
  

  Serial.println("LOOP");
}



void loop(void){

    
  SmartLocker.serverLoop();
  delay(2);
  

}