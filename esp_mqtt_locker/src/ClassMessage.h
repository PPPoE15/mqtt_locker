#ifndef CLASS_MESSAGE_H
#define CLASS_MESSAGE_H

#include <Arduino.h>
class Message
{
  public:
    Message(String type, byte plate, byte lock, byte* feedback){
      clearSerial();

      if(type == "open") { create_message(0x8A, plate, lock, 0x11);}
      else { create_message(0x80, plate, lock, 0x33); }

      Serial1.write(_message, 5);
      delay(50);
      Serial1.read(feedback, 11);
      delay(50);
    }

    Message(String type, byte plate, byte* feedback){
      clearSerial();

      if(type == "open") { create_message(0x8A, plate, 0, 0x11);}
      else { create_message(0x80, plate, 0, 0x33); }

      Serial1.write(_message, 5);
      delay(50);
      Serial1.read(feedback, 11);
      delay(50);
    }

    Message(String type, byte plate){
      clearSerial();
      if(type == "open") { create_message(0x8A, plate, 0, 0x11);}
      else { create_message(0x80, plate, 0, 0x33); }

      Serial1.write(_message, 5);
    }

  private:
    byte _message[5];
    void create_message(byte header, byte plate_addr, byte lock_addr, byte function_code){
      _message[0] = header;
      _message[1] = plate_addr;
      _message[2] = lock_addr;
      _message[3] = function_code;
      _message[4] = _message[0] ^ _message[1] ^ _message[2] ^ _message[3];
    }

    void clearSerial(){
      while(Serial1.available()){
        Serial1.read();
      }
      delay(50);
    }
};

#endif // CLASS_MESSAGE_H