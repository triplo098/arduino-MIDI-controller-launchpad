#ifndef LAU_MIDI_H
#define LAU_MIDI_H

#include <MIDIUSB.h>
#include "Arduino.h"
#include "config_settings.h"

class lau_midi {
    public:
        lau_midi();
        lau_midi(uint8_t _base_velocity, uint8_t _base_channel);
        static void noteOn(uint8_t note);
        static void noteOff(uint8_t note);
        static void controlChange(uint8_t control, uint8_t value);
        static void clean_midi();
        static void play_notes(bool on, uint8_t prime_note, scale& my_scale);
        static void note_on_time(uint8_t note);
        uint8_t base_velocity;
        uint8_t base_channel;

};



#endif