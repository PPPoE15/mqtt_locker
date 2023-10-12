#include <Arduino.h>

#define RXD2 18
#define TXD2 17

void setup() {
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  
}

byte message_unlock [] = {0x8A, 0x01, 0x00, 0x11, 0x9A};
byte message_lock []   = {0x8A, 0x01, 0x00, 0x00, 0x9A};

void loop() {
  delay(1000);                     
  Serial1.write(message_unlock, sizeof(message_unlock));
  delay(1000);                     
  Serial1.write(message_lock, sizeof(message_lock));
}