#ifndef LAN_KEYPAD_H
#define LAN_KEYPAD_H

#include "Keypad.h"
#include "scale.h"
#include "Arduino.h"
#include "lan_midi.h"
#include "config_settings.h"

class lan_keypad() : public Keypad {
    private:
        char keys[][];
        uint8_t rowPins[];  
        uint8_t colPins[];  
        void send_notes(bool key_active, uint8_t note);
        uint8_t change_mode_count;
        unsigned long keys_hold_timer;
    public:
        void initialize_keypad();
        void notes_to_keypad();
        void activate_keypad();

        
}


#endif
