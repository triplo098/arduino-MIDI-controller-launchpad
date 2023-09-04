#include "Mux.h"
#include <FastLED.h>
#include <Keypad.h>
#include "MIDIUSB.h"


#include "scale.h"
#include "config_settings.h"
#include "leds.h"
#include "lau_midi.h"

/*********************
Preparing multiplexer with potentiometers
*********************/
// using namespace admux;
// Mux mux(Pin(A0, INPUT, PinType::Analog), Pinset(16, 15, 14));

// #define NUMBER_OF_POTS 7

// struct potentiometer {

//   int mux_channel; 
//   int control_number;
//   int pre_value;
//   int value;
//   int channel = 0;
// };

// potentiometer potentiometers[NUMBER_OF_POTS];


/*********************
Preparing "music logic" and MIDI default parameters
*********************/
scale my_scale(60, MAJOR, 1);
lau_midi my_midi(100, 0)

config_settings congfig_settings(my_scale, SETUP_MODE, my_midi);

/*********************
Preparing LEDs
*********************/

leds leds;

/*********************
Preparing Keypad
*********************/
const uint8_t ROWS = 4;
const uint8_t COLS = 4;

char keys[ROWS][COLS];

uint8_t rowPins[ROWS] = {5, 4, 3, 2};
uint8_t colPins[COLS] = {9, 8, 7, 6}; 

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

/*********************
Default send-MIDI functions from MIDIUSB/examples/MIDIUSB_write/MIDIUSB_write.ino
*********************/



void setup() {


  // for(int i = 0; i < NUMBER_OF_POTS; i++) potentiometers[i].mux_channel = i;
  
  // potentiometers[0].control_number = 1;   //modulation
  // potentiometers[1].control_number = 2;   //random
  // potentiometers[2].control_number = 10;  //pan
  // potentiometers[3].control_number = 74;  //brigthness
  // potentiometers[4].control_number = 8;   //volume
  // potentiometers[5].control_number = 3;  //random
  // potentiometers[6].control_number = 4;  //random

  congfig_settings.load_from_EEPROM();

  leds.initialize_leds();

  Serial.begin(9600);

  notes_to_keypad();
}

void loop() {

  
  // activate_potentiometers();
      
  activate_keypad(congfig_settings.mode);

  if(congfig_settings.mode == AUTO_MODE) auto_mode();

  MidiUSB.flush();
  leds.update_leds(congfig_settings.mode);

}

// void activate_potentiometers() {
  
//   int mux_channel;
//   int value;
//   int control_number;

//   for(uint8_t i = 0; i < NUMBER_OF_POTS; i++) {
    
//     control_number = potentiometers[i].control_number;

//     mux_channel = potentiometers[i].mux_channel;

//     value = mux.read(mux_channel);

//     potentiometers[i].value = value;

//     //sending change of value only if there is a known change on potentiometer
//     if( potentiometers[i].value < potentiometers[i].pre_value + ACCEPTANCE_RATE
//     && potentiometers[i].value > potentiometers[i].pre_value - ACCEPTANCE_RATE ) continue; 
  
//     potentiometers[i].pre_value = potentiometers[i].value;

//     if( i == 3) FastLED.setBrightness(value / 4);

//     if(i == 6) leds.updates_per_second = value / 4;

//     //Sending midi control change
//     if(mode == NORMAL_MODE) {
//       controlChange(control_number, value / 8);
//       continue;
//     }

//     //Setting up parameters such as tonality, tonic, octave with potentiometers    
//     if( i == 1) {
//       if(value < 200) my_scale.set_tonality(MINOR_NAT);
//       else if(value < 400) my_scale.set_tonality(MINOR_HAR);
//       else if(value < 600) my_scale.set_tonality(MAJOR);
//       else if(value < 800) my_scale.set_tonality(PENTATONIC);
//       else if(value >= 800) my_scale.set_tonality(CHROMATIC);
//       notes_to_keypad();
//     }
    
//     if(i == 2) {
//       int temp_note = value / 8;
//       if(temp_note > 20 && temp_note <= 103) {
//         my_scale.set_tonic(temp_note);
//         //note_on_time(temp_note, 100);
//         notes_to_keypad();
//       }
//     }

//     if( i == 4) {
//       uint8_t chord_mode = (value / 256) + 1;
//       Serial.print("Mode: "), Serial.println(chord_mode);
//       switch(chord_mode) {
//         case 1: case 2: case 3: case 4:
//           my_scale.set_chord_notes(chord_mode);
//           break;
//         default:
//           my_scale.set_chord_notes(1);
//           break;
//       }  
//     }

//     if(i == 5) {
//       int octave = (value / 127);
//       int temp_note = (octave * 12) + (my_scale.get_tonic() % 12);
//       if(temp_note > 20 && temp_note <= 103 && temp_note != my_scale.get_tonic()) {
//         my_scale.set_tonic(temp_note);
//         //note_on_time(temp_note, 100);
//         notes_to_keypad();
//       }
//     }

//   }
// }


void activate_keypad(uint8_t& mode) {

  static uint8_t change_mode_count = 0;
  static long change_mode_time = 0;
  bool key_active = false;

  /*********************
  Modified part of code from Keypad/examples/MultiKey/MultiKey.ino
  *********************/
  if (kpd.getKeys()) {
    
    for (int i = 0; i < LIST_MAX; i++) {
       
      if ( kpd.key[i].stateChanged ) {

        int note = 0;
        uint8_t octave = 0;
        switch (kpd.key[i].kstate) {  

          // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
          case PRESSED:

            if(kpd.key[i].kchar == keys[3][3]) change_mode_count++;
            else if(kpd.key[i].kchar == keys[3][0]) change_mode_count++;

            if(change_mode_count == 2) {
              change_mode_time = millis();
              clean_midi();
              continue;
            } 
                
            note = (uint8_t) kpd.key[i].kchar;
            play_notes(true, note, my_scale);
        
            break;
          case HOLD:
          key_active = true;
            break;
          case RELEASED:

            if(kpd.key[i].kchar == keys[3][3] 
            || kpd.key[i].kchar == keys[3][0]) change_mode_count = 0;

            if(change_mode_count < 2) change_mode_time = 0;
            
            note = (uint8_t) kpd.key[i].kchar;
            
            play_notes(false, note, my_scale);

            break;
          case IDLE:
            break;
        }
      }      
    }
  }

  if((millis() - change_mode_time > 1000) && change_mode_time != 0) {
    
    mode++;
    mode %= 3;
    change_mode_time = 0;
    // set_color_palette(mode);
    // Serial.print("Mode: "), Serial.println(mode);
  }
}

void notes_to_keypad() {

  //Turning all of the notes from previou keypad off
  for(int i = 0 ; i < ROWS; i++) {
    for(int j = 0 ; j < COLS; j++) {
      noteOff((int)keys[i][j]);
    } 
  }
  
  Serial.print("Tonic: "), Serial.println(my_scale.get_tonic());
  Serial.print("Tonality: "), Serial.println(my_scale.get_tonality());

  int note_index = 0;
  int octave = 0;  
  for(int k = 0; k < ROWS; k++) {
    for(int n = 0; n < COLS; n++) {
      int acc_note = my_scale.get_tonic();

      acc_note += my_scale.notes[note_index] + (12*octave);

      keys[k][n] = acc_note;

      Serial.print((int)keys[k][n]), 
      Serial.print("  "); 
      
      note_index++;
      if(note_index == my_scale.count_notes()) octave++;
      note_index %= my_scale.count_notes();
    }
    Serial.println(); 
  }
  Serial.println();
}

void auto_mode() {

  static unsigned long play_time = millis();

  static uint8_t chord_index = 0;

  static uint8_t note_on = 0;
  
  int chords_number = 4;
  int chord_time = 2000; //time in ms

  
    //Controling likelihood of notes 
    uint8_t graviti_notes[] = {0, 0, 0, 2, 3, 3, 4, 4, 5, 1, 1, 2, 2, 6};
    //uint8_t graviti_notes[] = {0, 0, 3, 3};

    uint8_t note_index = graviti_notes[wheel_selection_index(sizeof(graviti_notes))]; 
    uint8_t note = my_scale.get_tonic() + my_scale.notes[note_index];
    
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

