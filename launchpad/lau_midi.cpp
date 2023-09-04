#include "lau_midi.h"

lau_midi::lau_midi() {}
lau_midi::lau_midi(uint8_t _base_velocity, uint8_t _base_channel) {
    base_velocity = _base_velocity;
    base_channel = _base_channel;
}
void lau_midi::noteOn(uint8_t note) {
  midiEventPacket_t noteOn = {0x09, 0x90 | base_channel, note, base_velocity};
  MidiUSB.sendMIDI(noteOn);
}

void lau_midi::noteOff(uint8_t note) {
  midiEventPacket_t noteOff = {0x08, 0x80 | base_channel, note, base_velocity};
  MidiUSB.sendMIDI(noteOff);
}

void lau_midi::controlChange(uint8_t control, uint8_t value) {
  midiEventPacket_t cc = {0x0B, 0xB0 | base_channel, control, value};
  MidiUSB.sendMIDI(cc);
}
void lau_midi::clean_midi() {

  //Turning all of the notes off
    for(int i = 0 ; i < 256; i++) noteOff(base_channel, i, base_velocity);
}

void lau_midi::play_notes(bool on, uint8_t prime_note, scale& my_scale) {

  int chords[my_scale.get_chord_notes()];

  uint8_t octave = 0;
  uint8_t index = 0;

  for(int i = 0; i < 3; i++){
    for(int j = 0; j < my_scale.count_notes(); j++) {
      if(prime_note == my_scale.get_tonic() + my_scale.notes[j] + (i * 12)) {
        octave = i;
        index = j;
        Serial.print("Note found: "), Serial.println(prime_note);
        Serial.print("Octave and index: "), Serial.print(octave), Serial.print(" "), Serial.println(index);
      }
    }
  } 

  for(int i = 0; i < my_scale.get_chord_notes(); i++) {
    if(index + i * 2 >= my_scale.count_notes()) octave = 1;
    else if(index + i * 2 >= 2*my_scale.count_notes()) octave = 2;
    chords[i] = my_scale.get_tonic() + my_scale.notes[(index + i * 2) % my_scale.count_notes()] + (12 * octave);
  }

  Serial.print("chord state: "), Serial.println(on);
  Serial.print("chord: ");

  for(int i = 0; i < my_scale.get_chord_notes(); i++) {
    Serial.print(chords[i]), Serial.print(" ");    
  }

  for(int note : chords) {
    if(on == true) {
      //Serial.print(note), Serial.print(" ");            
      noteOn(base_channel, note, base_velocity);

    } 
    else {
      //Serial.print(note), Serial.print(" ");            
      noteOff(base_channel, note, base_velocity);
    }
  }
  Serial.println();
}

//Not used yet. It's for playing chodrs without using delay
void note_on_time(uint8_t note) {

  static int time = 0;
  static unsigned long note_time = millis();
  static bool note_on = false;

  if(note_on == false) {
    noteOn(base_channel, note, base_velocity);
    note_on = true;
  } 
  else if((millis() - note_time) >= time) {
    noteOff(base_channel, note, base_velocity);
    note_on = false;
    note_time = millis();
  }
}
