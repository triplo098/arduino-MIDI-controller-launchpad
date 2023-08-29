#ifndef LAN_KEYPAD_H
#define LAN_KEYPAD_H

#include "Keypad.h"

class lan_keypad() : public Keypad {
    private:
        char keys[][];
        uint8_t rowPins[];  
        uint8_t colPins[];  
        unsigned long keys_hold_timer;
    public:
        void activate_keypad();
        void notes_to_keypad();
}


#endif
