#include "scale.h"

#define MAX_NOTES 12
#define MAJOR 'M'
#define MINOR_NAT 'm'
#define MINOR_HAR 'n'
#define PENTATONIC 'p'
#define CHROMATIC 'c'


scale::scale(uint8_t tonic, char tonality, uint8_t chord_notes) {

  for(int i = 0; i < MAX_NOTES; i++) notes[i] = 255;

  this -> tonic = tonic;
  this -> tonality = tonality;
  this -> chord_notes = chord_notes;

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

  uint8_t *temp_notes = nullptr;
  for(int i = 0; i < MAX_NOTES; i++) temp_notes[i] = 255;
  
  switch (tonality) {
  case 'n':
    temp_notes = new uint8_t[7]{0, 2, 3, 5, 7, 8, 10};
    break;
  case 'm':
    temp_notes = new uint8_t[7]{0, 2, 3, 5, 7, 8, 11};
    break;
  case 'M':
    temp_notes = new uint8_t[7]{0, 2, 4, 5, 7, 9, 11};
    break;
  case 'p':
    temp_notes = new uint8_t[5]{0, 2, 4, 7, 9};
    break;
  case 'c': default:
    for(int i = 0; i < MAX_NOTES; i++) temp_notes[i] = i;      
    break;
  }

  for(int i = 0; i < MAX_NOTES; i++) {
    if(temp_notes[i] == 255) break;
    notes[i] = temp_notes[i];
  }
  delete temp_notes;


}

uint8_t scale::get_tonic() {
  return tonic;
}

char scale::get_tonality() {
  return tonality;
}

uint8_t scale::get_chord_notes() {
  return chord_notes;
}