#ifndef EMG_BUTTON_H
#define EMG_BUTTON_H

#include "Config.h"

class button {
    public:
        button (byte pin) {
            _pin = pin;
            pinMode(_pin, INPUT_PULLUP);
            pinMode(LED_BUILTIN, OUTPUT);
        }
        
        bool click() {
            bool btnState = digitalRead(_pin);
            if (!btnState && !_flag && millis() - _tmr >= 100) {
                _flag = true;
                _tmr = millis();
                return true;
            }
            if (!btnState && _flag && millis() - _tmr >= 3000) {
                _tmr = millis ();
                emergencyOpen();
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
        void emergencyOpen(){
          for (int i=0; i < NUM_PLATES; i++){
            Message event2("open", i);
          }
        }
}emgButton(EMG_PIN);

#endif // EMG_BUTTON_H