#include "scale.h"

// void show_notes();

// int main () {
  
//   scale s(60, 'M', 3);
//   s.set_notes();

//   show_notes();
//   return 0;
// }

// void show_notes() {
//   for(int i = 0; i < s.count_notes(); i++) {
//     std::cout << "note[" << i << "]: " << (int) s.notes[i] << std::endl;
//   } 

//   std::cout << "Tonic: " << (int) s.get_tonic() << std::endl;
//   std::cout << "Tonality: " << s.get_tonality() << std::endl;
//   std::cout << "Chord Notes: " << (int) s.get_chord_notes() << std::endl;

//   std::cout << "Number of notes: " << (int) s.count_notes() << std::endl;

// }

scale::scale() {}

scale::scale(uint8_t tonic, char tonality, uint8_t chord_notes) {

  for(int i = 0; i < MAX_NOTES; i++) notes[i] = 255;

  this -> tonic = tonic;
  this -> tonality = tonality;
  this -> chord_notes = chord_notes;

}

uint8_t scale::count_notes() {

  uint8_t count = 0;
  for(int i = 0; i < MAX_NOTES; i++) {
    if(notes[i] != 255) count++;
  }

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

  switch (tonality) {
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