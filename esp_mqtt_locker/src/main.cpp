#include <Arduino.h>
#include <string.h>
#define RXD2 18
#define TXD2 17

void setup() {
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
}

byte message_unlock [] = {0x8A, 0x01, 0x01, 0x11, 0x9B};
byte recieved_data [5];
char check_data [5];

void loop() {                     
  Serial1.write(message_unlock, sizeof(message_unlock));
  Serial1.read(recieved_data, 5);


  if(recieved_data[3] == 0x11){  // successful unlocking
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
    delay(1000);                      // wait for a second
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  }
  
  delay(1000); 
               
}