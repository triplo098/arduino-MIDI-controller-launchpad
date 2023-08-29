#include "scale.h"

#define MAX_NOTES 12
#define MAJOR 'M'
#define MINOR_NAT 'm'
#define MINOR_HAR 'n'
#define PENTATONIC 'p'
#define CHROMATIC 'c'


scale::scale(uint8_t tonic, char tonality, uint8_t chord_notes) {

  for(int i = 0; i < MAX_NOTES; i++) notes[i] = 255;

  this.tonic = tonic;
  this.tonality = tonality;
  this.chord_notes = chord_notes;

}

uint8_t scale::count_notes() {

  uint8_t count = 0;
  for(int i = 0; i < MAX_NOTES; i++) if(notes[i] != 255) count++;
  return count;
}

void scale::set_tonic(uint8_t tonic) {

  this -> tonic = tonic;
  set_notes();

}

void scale::set_tonality(char tonality) {

  this -> tonality = tonality;
  set_notes();

}

void scale::set_chord_notes(uint8_t chord_notes) {

  this -> chord_notes = chord_notes;

}

void scale::set_notes() {

  for(int i = 0; i < MAX_NOTES; i++) notes[i] = 255;
  switch (scale::tonality) {
  case 'n':
    notes[0] = 0; notes[1] = 2;
    notes[2] = 3; notes[3] = 5;
    notes[4] = 7; notes[5] = 8;
    notes[6] = 10;
    break;
  case 'm':
    notes[0] = 0; notes[1] = 2; 
    notes[2] = 3; notes[3] = 5;
    notes[4] = 7; notes[5] = 8;
    notes[6] = 11;
    break;
  case 'M':
    notes[0] = 0; notes[1] = 2;
    notes[2] = 4; notes[3] = 5; 
    notes[4] = 7; notes[5] = 9;
    notes[6] = 11;
    break;
  case 'p':
    notes[0] = 0; notes[1] = 2;
    notes[2] = 4; notes[3] = 7;
    notes[4] = 9;
    break;
  case 'c': default:
    for(int i = 0; i < MAX_NOTES; i++) notes[i] = i;      
    break;
  } 

  for(int i = 0; i < MAX_NOTES; i++) notes[i] += tonic;

}
