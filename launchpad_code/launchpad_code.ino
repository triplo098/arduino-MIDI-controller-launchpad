

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

int base_velocity = 100;
int base_channel = 0;

unsigned long time = 0;
unsigned long start_time = 0;
unsigned long end_time = 0;

int minor_chord[] = {0, 3, 7};
int major_chord[] = {0, 4, 7};
int half_dim_chord[] = {0, 3, 6};


class scale {

  public:
  char tonality;
  byte tonic;
  byte notes[MAX_NOTES];
  byte count_notes = 0;
  byte chord_notes = 1;

  scale() {
    
    for(int i = 0; i < MAX_NOTES; i++) notes[i] = 255; //error message
    tonic = 60;
    tonality = MINOR_HAR;

    notes[0] = 0; notes[1] = 2;
    notes[2] = 4; notes[3] = 5; 
    notes[4] = 7; notes[5] = 9;
    notes[6] = 11;

    count_notes = 7;
  };
 
  void set_tonic(byte tonic) {
    
    this -> tonic = tonic;
  }

  void set_tonality(char tonality) {

    this -> tonality = tonality;

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
    count_notes = 0;
    //Serial.println("Notes: ");
    for(int i = 0; i < MAX_NOTES; i++) {
      //Serial.print(" "), Serial.print(notes[i]);
      if(notes[i] != 255) count_notes++;
    }
    
    //Serial.print("Count notes: "), Serial.println(count_notes);
  }

};

scale scale;



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
CRGB leds_layer[NUM_LEDS];

byte current_pot_index;

//preparing keypad
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS];


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


void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
  //Serial.print("NOTE ON: "), Serial.println(pitch);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
  //Serial.print("NOTE OFF: "), Serial.println(pitch);
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

#define SETUP_MODE 0
#define NORMAL_MODE 1
#define AUTO_MODE 2

int mode = NORMAL_MODE;

byte change_mode_count = 0;

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



void loop() {


  //midi_reading();

  activate_potentiometers(mode);
      
  activate_keypad(mode);
  MidiUSB.flush();
  FastLED.show();
}

void activate_potentiometers(int mode) {
  
  for(byte i = 0; i < number_of_pot; i++) {
    
    if(i == 0) continue;
    control_number = potentiometers[i].control_number;

    mux_channel = potentiometers[i].mux_channel; //multiplexer channel

    value = mux.read(mux_channel);

    potentiometers[i].value = value;

    //sending change of value only if there is a change on potentiometer
    if( potentiometers[i].value < potentiometers[i].pre_value + acceptance_rate
    && potentiometers[i].value > potentiometers[i].pre_value - acceptance_rate ) continue; 
  

    //color depends on number of potentiometer i, progress is used to turn on proper number of leds 
    //set_color_and_progress(i, ((double) potentiometers[i].value) / 1023.0 ); 
    if(mode == NORMAL_MODE) set_color_and_progress(i, value / 1023.0); 

    potentiometers[i].pre_value = potentiometers[i].value;
    
    
    if( i == 3) FastLED.setBrightness(value / 4);

    //Sending midi control change
    if(mode == NORMAL_MODE) {
      controlChange(channel, control_number, value / 8);
      continue;
    }

    //Setting up parameters such as tonality, tonic, octave with potentiometers    
    if( i == 1) {
      if(value < 200) scale.set_tonality(MINOR_NAT);
      else if(value < 400) scale.set_tonality(MINOR_HAR);
      else if(value < 600) scale.set_tonality(MAJOR);
      else if(value < 800) scale.set_tonality(PENTATONIC);
      else if(value >= 800) scale.set_tonality(CHROMATIC);
      notes_to_keypad();
    }
    
    if(i == 2) {
      int temp_note = value / 8;
      if(temp_note > 20 && temp_note <= 103) {
        scale.set_tonic(temp_note);
        notes_to_keypad();
      }
    }

    if( i == 4) {
      byte chord_mode = (value / 256) + 1;
      Serial.print("Mode: "), Serial.println(chord_mode);
      switch(chord_mode) {
        case 1: case 2: case 3: case 4:
          scale.chord_notes = chord_mode;
          break;
        default:
          scale.chord_notes = 1;
          break;
      }

      
    }

    if(i == 5) {
      int octave = (value / 127);
      int temp_note = (octave * 12) + (scale.tonic % 12);
      if(temp_note > 20 && temp_note <= 103) {
        scale.set_tonic(temp_note);
        notes_to_keypad();
      }
    }

  }

}


void activate_keypad(int &mode) {

  bool key_active = false;

  //snippet of code from examples from Keypad library repository
  if (kpd.getKeys()) {
    
    for (int i = 0; i < LIST_MAX; i++) {  // Scan the whole key list.
       
      if ( kpd.key[i].stateChanged ) {

        key_active = true;  // Only find keys that have changed state.
        int note = 0;
        byte octave = 0;
        switch (kpd.key[i].kstate) {  

          // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
          case PRESSED:

            if(kpd.key[i].kchar == keys[3][3]) change_mode_count++;
            else if(kpd.key[i].kchar == keys[3][0]) change_mode_count++;

            if(change_mode_count == 2) {
              start_time = millis();
              break;
            } 
            // else if(change_mode_count == 1) break;
            
            Serial.println(kpd.key[i].kchar);
            
            note = (byte) kpd.key[i].kchar;

            play_chord(true, note);
        
            //set_color_random();
            set_color_ametysth();
            break;
          case HOLD:
            break;
          case RELEASED:


            if(kpd.key[i].kchar == keys[3][3] 
            || kpd.key[i].kchar == keys[3][0]) change_mode_count = 0;

            if(change_mode_count < 2) start_time = 0;
            
            note = (byte) kpd.key[i].kchar;

            
            play_chord(false, note);

            //set_color_black();
            set_color_red();
            break;
          case IDLE:
            break;
        }
        Serial.print(kpd.key[i].kchar);
      }
      
    }
  }

  if((millis() - start_time > 2000) && start_time != 0) {
    
    mode++;
    mode %= 2;
    start_time = 0;

    Serial.print("Mode: "), Serial.println(mode);
  }
      //if(key_active == false) clean_midi();
}

void start_leds() {

  unsigned long starting_time = millis();
  byte brightness = 255;
  byte start_index = 0;
  
  do {
    byte color_index = start_index;

    for( int i = 0; i < NUM_LEDS; ++i) {

      leds[i] = ColorFromPalette(RainbowStripeColors_p, color_index, brightness, LINEARBLEND);
      color_index += 2;
    }
    FastLED.show();
    delay(10);
    Serial.println(start_index);
    start_index++;

  }
  while(millis() - starting_time < 2000);
}

//TO DO
void key_animation(bool on) {
  

}

void set_color_ametysth() {
  for(int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Amethyst;
}

void set_color_black() {
  for(int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Black;
}

void set_color_red() {
  for(int i = 0; i < NUM_LEDS; i++) leds[i].setRGB(233,150,122);
}


void set_color_and_progress(int pot_index, double progress) {
  int num_of_leds_on = (int) (NUM_LEDS * progress);

  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;

  //for(int i = 0; i < NUM_LEDS; i++) leds_layer[i] = leds[i];

  switch(pot_index) {
    case 0: 
      //pink
      red = 183; green = 28; blue = 28;
      break;
    case 1: 
      //light violet
      red = 69; green = 39; blue = 160;
      break;
    case 2: 
      //light cyan
      red = 67; green = 160; blue = 71;
      break;
    case 3: 
      //orange
      red = 245; green = 127; blue = 23;
      break;
    case 4: 
      //red brown
      red = 191; green = 54; blue = 12;
      break;
    // case 5: 
    //   //dark grey
    //   red = 93; green = 64; blue = 55;
    //   break;
    case 6: 
      //green
      red = 20; green = 230; blue = 20;
      break;
    default:
      red = 150; green = 64; blue = 210;
    break;
  }

  set_color_black();

  for(int i = 0; i < num_of_leds_on; i++) {
    leds[i].setRGB(red, green, blue);
  } 
}

void notes_to_keypad() {

  //Turning off all of the notes
  for(int i = 0 ; i < ROWS; i++) {
    for(int j = 0 ; j < COLS; j++) {
      noteOff(base_channel, (int)keys[i][j], base_velocity);
    } 
  }

  
  Serial.print("Tonic: "), Serial.println(scale.tonic);
  Serial.print("Tonality: "), Serial.println(scale.tonality);

  int note_index = 0;
  int octave = 0;  
  for(int k = 0; k < ROWS; k++) {
    for(int n = 0; n < COLS; n++) {
      int acc_note = scale.tonic;

      acc_note += scale.notes[note_index] + (12*octave);

      keys[k][n] = acc_note;

      Serial.print((int)keys[k][n]), 
      Serial.print("  "); 
      
      note_index++;
      if(note_index == scale.count_notes) octave++;
      note_index %= scale.count_notes;
    }
    Serial.println(); 
      // Serial.print("            Octave: "),
      // Serial.println(octave);
  }
  Serial.println();
}

void activate_auto_mode() {

  for(int note : scale.notes) {
    Serial.print(note), 
    Serial.print(" "); 
  }
  Serial.println();


  int chords_number = 4;
  int chord_notes = 3;
  int chord_time = 1000; //time in ms
  int chords_break_time = 1000; //time in ms
  

  // unsigned int start_time = millis();
  // unsigned int end_time = millis() + chord_time;

  //TO DO | PROPABLY BETTER IS TO USE POINTERS
  int chords[chords_number][chord_notes];
  
  for(int i = 0; i < chords_number; i++) {

    int prime = i;
    //int prime = notes[sizeof(a_min_har.notes)];

    // Serial.print("chord prime: "), Serial.println(prime);
    // Serial.print("tonic note: "), Serial.println(acc_scale.tonic);


    //Serial.print("Chord: ");
    for(int j = 0; j < chord_notes; j++) {
      //Serial.print("prime + 2 * j = "), Serial.println(prime + 2 * j);
      chords[i][j] = scale.notes[(prime + 2 * j) % 7] + scale.tonic;
      //Serial.print(chords[i][j]),
      //Serial.print(" ");
    }
    //Serial.println();

    start_time = millis();
    end_time = start_time + chord_time;


  }

  // Serial.println("Chords: ");
  // for(int i = 0; i < chords_number; i++) {
  //   for(int j = 0; j < chord_notes; j ++) {
  //     Serial.print(chords[i][j]), Serial.print(" ");
  //   }
  //   Serial.println();
  // }

  for(int i = 0; i < chords_number; i ++) {

      for(int j = 0; j < chord_notes; j++) noteOn(0, chords[i][j], base_velocity);
      delay(chord_time);

      for(int j = 0; j < chord_notes; j++) noteOff(0, chords[i][j], base_velocity);
      delay(chords_break_time);
  }


}

void clean_midi() {
  //Turning all of the notes off
    for(int i = 0 ; i < 256; i++) noteOff(base_channel, i, base_velocity);
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
void play_chord(bool on, byte prime_note) {

  int chords[scale.chord_notes];

  byte octave = 0;
  byte index = 0;

  for(int i = 0; i < 3; i++){
    for(int j = 0; j < scale.count_notes; j++) {
      if(prime_note == scale.tonic + scale.notes[j] + (i * 12)) {
        octave = i;
        index = j;
        Serial.print("Note found: "), Serial.println(prime_note);
        Serial.print("Octave and index: "), Serial.print(octave), Serial.print(" "), Serial.println(index);
      }
    }
  } 

  for(int i = 0; i < scale.chord_notes; i++) {
    if(index + i * 2 >= scale.count_notes) octave = 1;
    else if(index + i * 2 >= 2*scale.count_notes) octave = 2;
    chords[i] = scale.tonic + scale.notes[(index + i * 2) % scale.count_notes] + (12 * octave);
  }

  Serial.print("chord state: "), Serial.println(on);
  Serial.print("chord: ");

  for(int i = 0; i < scale.chord_notes; i++) {
    Serial.print(chords[i]), Serial.print(" ");    
  }

  for(int note : chords) {
    if(on == true) {
      //Serial.print(note), Serial.print(" ");            
      noteOn(base_channel, note, base_velocity);
    } else {
      //Serial.print(note), Serial.print(" ");            
      noteOff(base_channel, note, base_velocity);
    }
  }
  Serial.println();
}

void midi_reading() {

  unsigned long start_midi_reading = millis();

  int index = 0;

  midiEventPacket_t rx;
    // do {
    //   rx = MidiUSB.read();
    //   if (rx.header != 0) {
        
        // midi_signals[index].header = rx.header;
        // midi_signals[index].byte1 = rx.byte1;
        // midi_signals[index].byte2 = rx.byte2;
        // midi_signals[index].byte3 = rx.byte3;
        // midi_signals[index].time = millis() - start_midi_reading;

    //     Serial.print(rx.header), Serial.print("-"), Serial.print(rx.byte1), Serial.print("-");
    //     Serial.print(rx.byte2), Serial.print("-"), Serial.print(rx.byte3), Serial.print("-");
    //     Serial.println(midi_signals[index].time);
    //     index++;
    //   }
    // } while (index < 100 && rx.header != 0);

    do {
    rx = MidiUSB.read();
    if (rx.header != 0) {

      midi_signals[index].header = rx.header;
      midi_signals[index].byte1 = rx.byte1;
      midi_signals[index].byte2 = rx.byte2;
      midi_signals[index].byte3 = rx.byte3;
      midi_signals[index].time = millis() - start_midi_reading;

      Serial.print("Received: ");
      Serial.print(rx.header, HEX);
      Serial.print("-");
      Serial.print(rx.byte1, HEX);
      Serial.print("-");
      Serial.print(rx.byte2, HEX);
      Serial.print("-");
      Serial.println(rx.byte3, HEX);

      index++;
    }
  } while (rx.header != 0);
    // } while (millis() - start_midi_reading < 10000);

    // for(midi_signal signal : midi_signals) {

    //     Serial.print("Stored: ");
    //     Serial.print(signal.header, OCT);
    //     Serial.print("-");
    //     Serial.print(signal.byte1, OCT);
    //     Serial.print("-");
    //     Serial.print(signal.byte2, OCT);
    //     Serial.print("-");
    //     Serial.print(signal.byte3, OCT);
    //     Serial.print("---");
    //     Serial.println(signal.time, OCT);
    // }



}