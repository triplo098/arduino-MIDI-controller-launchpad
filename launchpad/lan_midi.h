#ifndef LAN_MIDI_H
#define LAN_MIDI_H

#include "MIDIUSB.h"
#include "Arduino.h"

class lan_midi : public MidiUSB() {

    public:
        void noteOn(uint8_t pitch);
        void noteOff(uint8_t pitch);
        void controlChange(uint8_t controlvalue);
        
    private:
        void base_channel;
        void base_velocity;

}
#endif