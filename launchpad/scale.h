#ifndef SCALE_H
#define SCALE_H

#include "stdint.h"

class scale {

  private:
    uint8_t tonic;    //represents first note of scale
    char tonality;    //tonality of the scale
    uint8_t chord_notes; //number of notes stacked up while sending MIDI signal
    void set_notes();

  public:
    scale(uint8_t tonic, char tonality, uint8_t chord_notes);
    uint8_t notes[];
    uint8_t count_notes();

    void set_tonic(uint8_t tonic);
    void set_tonality(char tonality);
    void set_chord_notes(uint8_t chord_notes);

    uint8_t get_tonic();
    char get_tonality();
    uint8_t get_chord_notes();
    
};

#endif
