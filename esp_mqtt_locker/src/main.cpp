#include "WiFiDevice.h"


WiFiDevice SmartLocker;

void infoHandler(){
  SmartLocker._server.send(200, "text/plain", "info handler: little bird says 'tweet'");
}

void setup(){
  Serial.begin(115200);
  SmartLocker.Init("smart_locker", "12345678");
  SmartLocker.addHandler("/info", infoHandler);
  

  Serial.println("LOOP");
}



void loop(){

    
  SmartLocker.serverLoop();
  delay(2);
  

}