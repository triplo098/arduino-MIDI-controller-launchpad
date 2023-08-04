/*
  pot = potentiometers

*/

#include "MIDIUSB.h"

#include <Keypad.h>

#include "Mux.h"
#include <Arduino.h>

#include <FastLED.h>

using namespace admux;

#define MAX_NOTES 12
#define MAJOR 'M'
#define MINOR_NAT 'm'
#define MINOR_HAR 'n'
#define PENTATONIC 'p'
#define CHROMATIC 'c'

int notes[MAX_NOTES];
int tonic_note = 60; //middle c
char tonality = 'n'; //Natural minor scale
int base_velocity = 100;
int base_channel = 0;

unsigned long time;
unsigned long start_time;
unsigned long end_time;

  int minor_chord[] = {0, 3, 7};
  int major_chord[] = {0, 4, 7};
  int half_dim_chord[] = {0, 3, 6};

// struct scale {
//   char s_tonality;
//   byte s_tonic_note;

//   byte notes[7];
//   int notes_gravity[7];


// };

class scale {
  char s_tonality;
  byte s_tonic;

  byte notes[7];
  int notes_gravity[7];

  public: scale(char s_tonality, byte s_tonic) {

    this -> s_tonality = s_tonality;
    this -> s_tonic = s_tonic;

    switch (s_tonality) {
      case 'M':
      notes[0] = 0; notes[1] = 2;
      notes[2] = 4; notes[3] = 5; 
      notes[4] = 7; notes[5] = 9;
      notes[6] = 11;
      break;
    case 'n':
      notes[0] = 0; notes[1] = 2;
      notes[2] = 3; notes[3] = 5;
      notes[4] = 7; notes[5] = 8;
      notes[6] = 10;
      break;
    default:
      notes[0] = 0; notes[1] = 2; 
      notes[2] = 3; notes[3] = 5;
      notes[4] = 7; notes[5] = 8;
      notes[6] = 11;
      break;
    }

  }

};

struct midi_signal {
	uint8_t header;
	uint8_t byte1;
	uint8_t byte2;
	uint8_t byte3;
  unsigned long time;
}; 

midi_signal midi_signals[100];

//Mux mux(Pinset(5, 6, 7));

Mux mux(admux::Pin(A0, INPUT, PinType::Analog), Pinset(15, 14, 16));


#define LED_PIN 10
#define NUM_LEDS 16

CRGB leds[NUM_LEDS];

//preparing keypad
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'<','=','>', '?'},
  {'@','A','B', 'C'},
  {'D','E','F', 'G'},
  {'H','I','J', 'K'}
};

byte rowPins[ROWS] = {5, 4, 3, 2};
byte colPins[COLS] = {9, 8, 7, 6}; 
// byte rowPins[ROWS] = {2, 3, 4, 5};
// byte colPins[COLS] = {6, 7, 8, 9}; 

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );



//
//potentiometers
//
const byte number_of_pot = 7;

struct potentiometer {

  int mux_channel; 
  int control_number;
  int pre_value;
  int value;
  int channel = 0;

};

potentiometer potentiometers[number_of_pot];


//buttons
//TO DO 


// const int button1Pin = 10;
// int button1State = 0;


void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
  //Serial.println(pitch);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS); 
  FastLED.setBrightness(100);

  start_leds();

  for(int i = 0; i < number_of_pot; i++) potentiometers[i].mux_channel = i;
  
  potentiometers[0].control_number = 1;   //modulation
  potentiometers[1].control_number = 2;   //random
  potentiometers[2].control_number = 10;  //pan
  potentiometers[3].control_number = 74;  //brigthness
  potentiometers[4].control_number = 8;   //volume
  potentiometers[5].control_number = 3;  //random
  potentiometers[6].control_number = 4;  //random

  
}

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

int mux_channel;
int pre_value;
int value;
int control_number;
int channel;
int acceptance_rate = 20; //value on potentiometer that need to
                          //be changed to send midi command


void loop() {
  //midi_reading();

  
  //controloing potentiometers
  for(byte i = 0; i < number_of_pot; i++) {
    
    if(i == 0) continue;
    control_number = potentiometers[i].control_number;

    mux_channel = potentiometers[i].mux_channel; //multiplexer channel

    value = mux.read(mux_channel);

    potentiometers[i].value = value;

    //sending change of value only if there is a change on potentiometer
    if( potentiometers[i].value < potentiometers[i].pre_value + acceptance_rate
    && potentiometers[i].value > potentiometers[i].pre_value - acceptance_rate ) continue; 
  
    //printing to serial and testing
    //Serial.print("Pot: "); Serial.print(i); Serial.print(" - "); Serial.println(value);

    //set_color_random(); 
    //color depends on number of potentiometer i, progress is used to turn on proper number of leds 
    //set_color_and_progress(i, ((double) potentiometers[i].value) / 1023.0 ); 
    set_color_and_progress(i, 1.0); 

    //Sending midi control change
    controlChange(channel, control_number, value / 8);
    
    potentiometers[i].pre_value = potentiometers[i].value;

    //Special potentiometers features
    if( i == 3) FastLED.setBrightness(value / 4);
    //FastLED.setBrightness(potentiometers[i].value / 4);


    //if( i == 6) set_octave(value);
    
    if(i == 2) {
      int temp_note = value / 8;
      if(temp_note > 20 && temp_note <= 103) tonic_note = temp_note;
      //Serial.println(tonic_note);
    }

    if( i == 1) {
      if(value < 200) set_scale_and_tonality(tonic_note, MINOR_NAT);
      else if(value < 400) set_scale_and_tonality(tonic_note, MINOR_HAR);
      else if(value < 600) set_scale_and_tonality(tonic_note, MAJOR);
      else if(value < 800) set_scale_and_tonality(tonic_note, PENTATONIC);
      else if(value >= 800) set_scale_and_tonality(tonic_note, CHROMATIC);
    }


  }

  // if(potentiometers[0].value < 10 && potentiometers[1].value < 10 ) auto_mode(true);
  // else auto_mode(false);

  // set_scale_and_tonality(60, MAJOR);

  // int chords_number = 4;
  // int chord_notes = 3;

  // int chords[chords_number][chord_notes] = {{0, 4, 7}, {0, 5, 9}, {0, 4, 7}, {0, 5, 9}};
  // for(int i = 0; i < chords_number; i++) {

  //   for(int j = 0; j < chord_notes; j++) {
  //       chords[i][j] += (tonic_note);
  //     noteOn(0, chords[i][j], 100);
  //     }
  //   for(int j = 0; j < chord_notes; j++) noteOff(0, chords[i][j], 100);
  // }

  // play_chord(1, 'm', 0);
  // delay(1000);
  // play_chord(0, 'm', 0);
  //auto_mode(true);
  //Serial.println("");

  //delay(1000);
  
  //controling notes
  if (kpd.getKeys()) {
    for (int i=0; i < LIST_MAX; i++) {  // Scan the whole key list.
      if ( kpd.key[i].stateChanged ) {  // Only find keys that have changed state.
        int note = 0;
        switch (kpd.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
          case PRESSED:
            Serial.println(kpd.key[i].kchar);
            note = (int) kpd.key[i].kchar;
            noteOn(0, note, 127);
            Serial.print("Note On ");
            Serial.println(note);

            //set_color_random();
            set_color_ametysth();
            break;
          case HOLD:
            break;
          case RELEASED:
            note = (int) kpd.key[i].kchar;
            noteOff(0, note, 127);
            Serial.print("Note Off ");
            Serial.println(note);
            //set_color_black();
            break;
          case IDLE:
            break;
        }
        Serial.print(kpd.key[i].kchar);
      }
    }
  }

  MidiUSB.flush();
  FastLED.show();

  // int count_0 = 0; 
  // int count_1 = 0; 
  // int count_2 = 0; 
  // for(int i = 0; i < 100; i++) {
  //   int value = wheel_selection_index(4);
  //   Serial.print("TEST "), Serial.print(i + 1), Serial.print(": "),
  //   Serial.println(value);

  //   if(value == 0) count_0++;
  //   else if(value == 1) count_1++;
  //   else if(value == 2) count_2++;
  // }
  // Serial.print("Zeros: "), Serial.println(count_0);
  // Serial.print("Ones: "), Serial.println(count_1);
  // Serial.print("Twos: "), Serial.println(count_2);

  // delay(1000);
}

void start_leds() {

  int change_red = 0;
  int change_green = 0;
  int change_blue = 0;

  int diffrence_in_color = 31;
  for(int i = 0; i < NUM_LEDS; i++) {

    leds[i] = CRGB (50, change_green, 50);
      
    change_red += diffrence_in_color;
    change_green += diffrence_in_color;
    change_blue += diffrence_in_color;
    FastLED.show();
    delay(100);
  }
  
}

void set_color_ametysth() {
  for(int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Amethyst;
}

void set_color_black() {
  for(int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Black;
}


void set_color_and_progress(int pot_index, double progress) {
  int num_of_leds_on = (int) (NUM_LEDS * progress);

  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;

  switch(pot_index) {
    case 0: 
      //pink
      red = 183;
      green = 28;
      blue = 28;
      break;
    case 1: 
      //light violet
      red = 69;
      green = 39;
      blue = 160;
      break;
    case 2: 
      //light cyan
      red = 67;
      green = 160;
      blue = 71;
      break;
    case 3: 
      //orange
      red = 245;
      green = 127;
      blue = 23;
      break;
    case 4: 
      //red brown
      red = 191;
      green = 54;
      blue = 12;
      break;
    // case 5: 
    //   //dark grey
    //   red = 93;
    //   green = 64;
    //   blue = 55;
    //   break;
    case 6: 
      //green
      red = 20;
      green = 230;
      blue = 20;
      break;
    default:
      red = 150;
      green = 64;
      blue = 210;
    break;
  }

  for(int i = 0; i < num_of_leds_on; i++) {
    leds[i].setRGB(red, green, blue);
  } 
  FastLED.show();
}

void set_octave(int pot_value) { 

      int choose_midi_note = 24 + ((8 * pot_value) / 1023) * 12; 

      //Serial.println(choose_midi_note);
      for(int k = 0; k < ROWS; k++) {
        for(int n = 0; n < COLS; n++) {
          keys[k][n] = choose_midi_note;
          // Serial.println(keys[k][n]);
          choose_midi_note++;
        }
      }
      Serial.println();
}


//tonality m = minor natural
//tonality M = major natural
//tonality p = pentatonic
void set_scale_and_tonality(int tonic_note, char tonality) {
  
  for(int i = 0; i < MAX_NOTES; i++) notes[i] = -1;
  switch(tonality) {
    case 'M':
      notes[0] = 0;
      notes[1] = 2;
      notes[2] = 4;
      notes[3] = 5;
      notes[4] = 7;
      notes[5] = 9;
      notes[6] = 11;
      break;
    case 'm':
      notes[0] = 0;
      notes[1] = 2;
      notes[2] = 3;
      notes[3] = 5;
      notes[4] = 7;
      notes[5] = 8;
      notes[6] = 10;
      break;
    case 'n':
      notes[0] = 0;
      notes[1] = 2;
      notes[2] = 3;
      notes[3] = 5;
      notes[4] = 7;
      notes[5] = 8;
      notes[6] = 11;
      break;
    case 'p':
      notes[0] = 0;
      notes[1] = 2;
      notes[2] = 4;
      notes[3] = 7;
      notes[4] = 9;
      break;
    case 'c':
      for(int i = 0; i < MAX_NOTES; i++) notes[i] = i;      
      break;
    default:
      for(int i = 0; i < MAX_NOTES; i++) notes[i] = i;      
      break;

  }
  int count_notes = 0;
  for(int i =0; i < MAX_NOTES; i++) 
    if(notes[i] != -1) count_notes++;
  
  // for(int i = 0; i < count_notes; i++) {
  //   noteOn(0, tonic_note + notes[i], 99);
  //   delay(500);
  //   noteOff(0, tonic_note + notes[i], 99);
  //   delay(500);
  //   MidiUSB.flush();
  // }
  

    Serial.print("Count notes:");
    Serial.println(count_notes);
    for(int i = 0; i < count_notes; i++) {
      Serial.print("Note "), Serial.print(i), 
      Serial.print(": "), Serial.println(notes[i]); 
    }
    Serial.println();

    Serial.println("tonic note + tonality:"),
    Serial.print(tonic_note), Serial.println(tonality);

  
    int note_index = 0;
    int octave = 0;  
    for(int k = 0; k < ROWS; k++) {
      for(int n = 0; n < COLS; n++) {
        int acc_note = tonic_note;
 
        acc_note += notes[note_index] + (12*octave);

        keys[k][n] = acc_note;

        Serial.print((int)keys[k][n]), 
        Serial.print("  "); 
        
        note_index++;
        if(note_index == count_notes) octave++;
        note_index %= count_notes;
      }
        Serial.println(); 
        // Serial.print("            Octave: "),
        // Serial.println(octave);
    }
  Serial.println();

  //TO DO
  // int *temp_notes = new int[count_notes];
  // int *ptr;

  // for(int i = 0; i < count_notes; i++) temp_notes[i] = notes[i];
  // notes = realloc( *notes, count_notes * sizeof(int));

  //   for(int i = 0; i < count_notes; i++) notes[i] = temp_notes[i];

  //   free(notes);



  // delete[] notes; 
  // notes = temp_notes;

}

void auto_mode(bool on) {

  if(on == false) return 0;

  set_scale_and_tonality(60, MAJOR);

  scale a_min_har('m', 57);

  int chords_number = 4;
  int chord_notes = 3;
  int chord_time = 1000; //time in ms
  int chords_break_time = 1000; //time in ms
  

  // unsigned int start_time = millis();
  // unsigned int end_time = millis() + chord_on_time;

  //TO DO | PROPABLY BETTER IS TO USE POINTERS
  int chords[chords_number][chord_notes];

  for(int i = 0; i < chords_number; i++) {

    int prime = notes[0];

    Serial.print("chord prime: "), Serial.println(prime);
    Serial.print("tonic note: "), Serial.println(tonic_note);


    switch(prime) {
      case 0: case 5: case 7:
      play_chord(true, 'M', prime);
        break;
      case 2: case 3: case 9:
      play_chord(true, 'm', prime);
        break;
      case 11:
        play_chord(true, 'd', prime);
        break;
      defalut:
        break;
    }


    Serial.print("Chord: ");
    for(int j = 0; j < chord_notes; j++) {
      chords[i][j] += prime + tonic_note;
      Serial.print(chords[i][j]),
      Serial.print(" ");
    }
    Serial.println();

    start_time = millis();
    end_time = start_time + chord_time;


  }


  //   for(int j = 0; j < chord_notes; j++) noteOn(0, chords[0][j], 70);
  //   delay(chord_on_time);

  //   for(int j = 0; j < chord_notes; j++) noteOff(0, chords[0][j], 70);
  //   delay(chords_break_time);
  // Serial.println();


}


int wheel_selection_index(int size) {
                          
  int sum = 0; 
  for(int i = 0; i < size; i++) sum += (size - i); 
  int random_number = (rand() % sum) + 1; 

  int cumulative_sum = 0;

  for (int i = 0; i < size; i++) {
        
      cumulative_sum += (size - i); 

      if(random_number <= cumulative_sum) return i;
  }

}

//ch_symbol = chord symbol, ch_notes = chord_notes which means, what notes are in chord 
//length is in milliseconds
void play_chord(bool on, char ch_symbol, int prime) {

  int size = 3;
  int chord[size];

  switch(ch_symbol) {
    case 'm':
      for(int i = 0; i < size; i++) chord[i] = minor_chord[i];
      break;
    case 'M':
      for(int i = 0; i < size; i++) chord[i] = major_chord[i];
      break;
    case 'd':
      for(int i = 0; i < size; i++) chord[i] = half_dim_chord[i];
      break;
    default:
      break;
  }
  for(int i = 0; i < size; i++) chord[i] += (prime + tonic_note);

  Serial.print("chord state: "), Serial.println(on);
  Serial.print("chord: ");

  if(on == true) 
    for(int note : chord) {
      Serial.print(note), Serial.print(" ");
      noteOn(base_channel, note, base_velocity);
    }
  else for(int note : chord) noteOff(base_channel, note, base_velocity);

  Serial.println();

}

void midi_reading() {

  unsigned long start_midi_reading = millis();

  int index = 0;

  midiEventPacket_t rx;
    do {
      rx = MidiUSB.read();
      if (rx.header != 0 && rx.byte1 >= 128 && rx.byte1 < 160) {
        
        midi_signals[index].header = rx.header;
        midi_signals[index].byte1 = rx.byte1;
        midi_signals[index].byte2 = rx.byte2;
        midi_signals[index].byte3 = rx.byte3;
        midi_signals[index].time = millis() - start_midi_reading;
        index++;
      }
    } while (millis() - start_midi_reading < 10000);

    for(midi_signal signal : midi_signals) {

        Serial.print("Stored: ");
        Serial.print(signal.header, OCT);
        Serial.print("-");
        Serial.print(signal.byte1, OCT);
        Serial.print("-");
        Serial.print(signal.byte2, OCT);
        Serial.print("-");
        Serial.print(signal.byte3, OCT);
        Serial.print("---");
        Serial.println(signal.time, OCT);
    }

    delay(5000);

}