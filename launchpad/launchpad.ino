#include "MIDIUSB.h"

#include <Keypad.h>

#include "Mux.h"


#include <FastLED.h>


/*********************
Preparing multiplexer with potentiometers
*********************/
using namespace admux;
Mux mux(admux::Pin(A0, INPUT, PinType::Analog), Pinset(16, 15, 14));

#define NUMBER_OF_POTS 7

struct potentiometer {

  int mux_channel; 
  int control_number;
  int pre_value;
  int value;
  int channel = 0;
};

potentiometer potentiometers[NUMBER_OF_POTS];

int acceptance_rate = 32; //analong value in range betweet 0 and 1023       


/*********************
Preparing "music logic" and MIDI default parameters
*********************/
#define MAX_NOTES 12
#define MAJOR 'M'
#define MINOR_NAT 'm'
#define MINOR_HAR 'n'
#define PENTATONIC 'p'
#define CHROMATIC 'c'

byte base_velocity = 100;
byte base_channel = 0;

class scale {

  public:
  char tonality; //determine what intervals are between notes
  byte tonic;    //represents first note of scale
  byte notes[MAX_NOTES];
  byte chord_notes = 1; //number of notes stacked up while sending MIDI signal

  scale() {

    //initializing notes with error message
    for(int i = 0; i < MAX_NOTES; i++) notes[i] = 255;
    tonic = 60;
    tonality = MINOR_HAR;

    notes[0] = 0; notes[1] = 2;
    notes[2] = 4; notes[3] = 5; 
    notes[4] = 7; notes[5] = 9;
    notes[6] = 11;
  };
  
  byte count_notes() {

    byte count = 0;
    for(int i = 0; i < MAX_NOTES; i++) if(notes[i] != 255) count++;
    return count;
  }

  void set_tonic(byte tonic) {

    this -> tonic = tonic;
  }

  void set_tonality(char tonality) {

    this -> tonality = tonality;

    //error message
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
};

scale scale;


/*********************
Preparing LEDs
*********************/
#define LED_PIN 10
#define NUM_LEDS 15

int updates_per_second = 50;

CRGB leds[NUM_LEDS];

CRGBPalette16 leds_palette;
TBlendType leds_blend;

unsigned long leds_timer = 0;


/*********************
Preparing Keypad
*********************/
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS];

byte rowPins[ROWS] = {5, 4, 3, 2};
byte colPins[COLS] = {9, 8, 7, 6}; 

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

unsigned long keys_hold_time = 0;


/*********************
Default send-MIDI functions from MIDIUSB/examples/MIDIUSB_write/MIDIUSB_write.ino
*********************/
void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}


#define SETUP_MODE 0
#define NORMAL_MODE 1
#define AUTO_MODE 2 //TO DO - not implemented yet
unsigned long auto_mode_timer;

byte mode;
byte change_mode_count = 0;

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS); 
  FastLED.setBrightness(100);

  leds_palette = RainbowStripeColors_p;
  leds_blend = LINEARBLEND;

  start_leds();

  for(int i = 0; i < NUMBER_OF_POTS; i++) potentiometers[i].mux_channel = i;
  
  potentiometers[0].control_number = 1;   //modulation
  potentiometers[1].control_number = 2;   //random
  potentiometers[2].control_number = 10;  //pan
  potentiometers[3].control_number = 74;  //brigthness
  potentiometers[4].control_number = 8;   //volume
  potentiometers[5].control_number = 3;  //random
  potentiometers[6].control_number = 4;  //random

  leds_timer = millis();

  mode = AUTO_MODE;
  set_color_palette(mode); 
}

void loop() {

  static byte leds_start_index = 0;

  activate_potentiometers(mode);
      
  activate_keypad(mode);

  if(mode == AUTO_MODE) auto_mode();


  MidiUSB.flush();

  if((millis() - leds_timer) > (1000 / updates_per_second)) {

    fill_leds_palette(leds_start_index);
    FastLED.show();
    leds_start_index++;
    leds_timer = millis();
  }
}

void activate_potentiometers(byte mode) {
  
  int mux_channel;
  int value;
  int control_number;

  for(byte i = 0; i < NUMBER_OF_POTS; i++) {
    
    control_number = potentiometers[i].control_number;

    mux_channel = potentiometers[i].mux_channel;

    value = mux.read(mux_channel);

    potentiometers[i].value = value;

    //sending change of value only if there is a known change on potentiometer
    if( potentiometers[i].value < potentiometers[i].pre_value + acceptance_rate
    && potentiometers[i].value > potentiometers[i].pre_value - acceptance_rate ) continue; 
  
    //set_color_and_progress(i, ((double) potentiometers[i].value) / 1023.0 ); 

    potentiometers[i].pre_value = potentiometers[i].value;

    if( i == 3) FastLED.setBrightness(value / 4);

    if(i == 6) updates_per_second = value / 4;

    //Sending midi control change
    if(mode == NORMAL_MODE) {
      controlChange(base_channel, control_number, value / 8);
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
        //note_on_time(temp_note, 100);
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
      if(temp_note > 20 && temp_note <= 103 && temp_note != scale.tonic) {
        scale.set_tonic(temp_note);
        //note_on_time(temp_note, 100);
        notes_to_keypad();
      }
    }

  }
}


void activate_keypad(byte &mode) {

  bool key_active = false;

  /*********************
  Modified part of code from Keypad/examples/MultiKey/MultiKey.ino
  *********************/
  if (kpd.getKeys()) {
    
    for (int i = 0; i < LIST_MAX; i++) {
       
      if ( kpd.key[i].stateChanged ) {

        int note = 0;
        byte octave = 0;
        switch (kpd.key[i].kstate) {  

          // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
          case PRESSED:

            if(kpd.key[i].kchar == keys[3][3]) change_mode_count++;
            else if(kpd.key[i].kchar == keys[3][0]) change_mode_count++;

            if(change_mode_count == 2) {
              keys_hold_time = millis();
              clean_midi();
              continue;
            } 
                
            note = (byte) kpd.key[i].kchar;
            play_notes(true, note);
        
            break;
          case HOLD:
          key_active = true;
            break;
          case RELEASED:

            if(kpd.key[i].kchar == keys[3][3] 
            || kpd.key[i].kchar == keys[3][0]) change_mode_count = 0;

            if(change_mode_count < 2) keys_hold_time = 0;
            
            note = (byte) kpd.key[i].kchar;
            
            play_notes(false, note);

            break;
          case IDLE:
            break;
        }
      }      
    }
  }

  if((millis() - keys_hold_time > 1000) && keys_hold_time != 0) {
    
    mode++;
    mode %= 3;
    keys_hold_time = 0;
    set_color_palette(mode);
    Serial.print("Mode: "), Serial.println(mode);
  }
}

void start_leds() {

  unsigned long starting_time = millis();
  byte leds_start_index = 0;
  
  do {
    
    fill_leds_palette(leds_start_index);
    FastLED.show();
    delay(10);
    // Serial.println(leds_start_index);
    leds_start_index++;
  }
  while(millis() - starting_time < 2000);
}

void fill_leds_palette(byte color_index) {
    
  byte brightness = 255;

  for( int i = 0; i < NUM_LEDS; ++i) {

    leds[i] = ColorFromPalette(leds_palette, color_index, brightness, leds_blend);
    color_index += 2;
  }
}

void set_color_palette(byte mode) {
  switch(mode) {
    case 0: default:
    leds_palette = ForestColors_p;
      break;
    case 1:
    leds_palette = CloudColors_p;
      break;
    case 2:
    leds_palette = OceanColors_p;
      break;
  };
}

void set_color_black() {
  for(int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Black;
}

//color depends on number of potentiometer i, progress is used to turn on proper number of leds 
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

  //Turning all of the notes from previou keypad off
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
      if(note_index == scale.count_notes()) octave++;
      note_index %= scale.count_notes();
    }
    Serial.println(); 
  }
  Serial.println();
}

void auto_mode() {

  static unsigned long play_time = millis();

  static byte chord_index = 0;

  static byte note_on = 0;
  
  int chords_number = 4;
  int chord_time = 2000; //time in ms

  
    //Controling likelihood of notes 
    byte graviti_notes[] = {0, 0, 0, 2, 3, 3, 4, 4, 5, 1, 1, 2, 2, 6};
    //byte graviti_notes[] = {0, 0, 3, 3};

    byte note_index = graviti_notes[wheel_selection_index(sizeof(graviti_notes))]; 
    byte note = scale.tonic + scale.notes[note_index];
    
    if(note_on == 0) {
      play_notes(true, note);
      note_on = note;
    }

    if(millis() - play_time > chord_time) {
      play_notes(false, note);
      note_on = 0;
      play_time = millis();
    }

    Serial.print("Prime NOTE: "), Serial.println(note);

    if(chord_index  >= chords_number) chord_index %= chords_number;
  

  Serial.println();

  // Serial.println("Chords: ");
  // for(int i = 0; i < chords_number; i++) {
  //   for(int j = 0; j < chord_notes; j ++) {
  //     Serial.print(chords[i][j]), Serial.print(" ");
  //   }
  //   Serial.println();
  // }
}

void clean_midi() {

  //Turning all of the notes off
    for(int i = 0 ; i < 256; i++) noteOff(base_channel, i, base_velocity);
}

//Not used yet. It's should be useful while generating chords
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

void play_notes(bool on, byte prime_note) {

  if(on == true ) leds_palette = LavaColors_p;
  else set_color_palette(mode);

  int chords[scale.chord_notes];

  byte octave = 0;
  byte index = 0;

  for(int i = 0; i < 3; i++){
    for(int j = 0; j < scale.count_notes(); j++) {
      if(prime_note == scale.tonic + scale.notes[j] + (i * 12)) {
        octave = i;
        index = j;
        Serial.print("Note found: "), Serial.println(prime_note);
        Serial.print("Octave and index: "), Serial.print(octave), Serial.print(" "), Serial.println(index);
      }
    }
  } 

  for(int i = 0; i < scale.chord_notes; i++) {
    if(index + i * 2 >= scale.count_notes()) octave = 1;
    else if(index + i * 2 >= 2*scale.count_notes()) octave = 2;
    chords[i] = scale.tonic + scale.notes[(index + i * 2) % scale.count_notes()] + (12 * octave);
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

//Not used yet. It's for playing chodrs without using delay
void note_on_time(byte note, int timer) {

  static int time = timer;
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