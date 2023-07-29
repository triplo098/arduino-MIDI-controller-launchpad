/*
  pot = potentiometers

*/

#include "MIDIUSB.h"

#include <Keypad.h>

#include "Mux.h"
#include <Arduino.h>

#include <FastLED.h>

using namespace admux;

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


String msg;

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
  potentiometers[1].control_number = 7;   //volume
  potentiometers[2].control_number = 10;  //pan
  potentiometers[3].control_number = 74;  //brigthness
  potentiometers[4].control_number = 2;   //random
  potentiometers[5].control_number = 3;  //random
  potentiometers[6].control_number = 4;  //random

  msg = "";

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
int acceptance_rate = 30;


void loop() {

  //start_leds();


  // int data;
  // for (byte i = 0; i < 4; i++) {
  //   data = mux.read(i) /* Reads from channel i (returns a value from 0 to 1023) */;

  //   Serial.print("Potentiometer at channel "); Serial.print(i); Serial.print(" is at "); Serial.print((double) (data) * 100 / 1023); Serial.println("%%");
  // }
  // Serial.println();

  // delay(1500);

  //controloing potentiometers
  for(byte i = 0; i < number_of_pot; i++) {
    
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
    set_color_and_progress(i, ((double) potentiometers[i].value) / 1023.0 ); //color depends on number of potentiometer i, progress is used to turn on proper number of leds 

    //sending midi control change
    controlChange(channel, control_number, value / 8);
    
    potentiometers[i].pre_value = potentiometers[i].value;

    if( i == 3) FastLED.setBrightness(potentiometers[i].value / 4);

    // if( i == 6) { 
    //   int choose_note = 24 + ((8 * potentiometers[i].value) / 1023) * 12; 

    //   // Serial.println(choose_note);
    //   for(int k = 0; k < (ROWS * COLS); k++) {
    //       // keys[k][n] = choose_note + '0';
    //       kpd.key[k].kchar = choose_note + '0';
    //       Serial.println(kpd.key[k].kchar);
    //       choose_note++;

    //   }
      Serial.println();
    }

 
  }

  //Serial.println("");

  //delay(1000);
  
  //controling notes
  if (kpd.getKeys())
  {
    for (int i=0; i < LIST_MAX; i++)   // Scan the whole key list.
    {
      if ( kpd.key[i].stateChanged )   // Only find keys that have changed state.
      {
        int note = 0;
          switch (kpd.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
              case PRESSED:
              msg = " PRESSED.";

              Serial.println(kpd.key[i].kchar);
              note = kpd.key[i].kchar;
              noteOn(0, note, 127);
              Serial.print("Note On");
              Serial.println(note);

              //set_color_random();
              set_color_ametysth();
             
          break;
              case HOLD:
              msg = " HOLD.";
          break;
              case RELEASED:
              msg = " RELEASED.";

              note = kpd.key[i].kchar;
              noteOff(0, note, 127);
              Serial.print("Note Off");
              Serial.println(note);
              //set_color_black();

          break;
              case IDLE:
              msg = " IDLE.";
          }
          Serial.print("Key ");
          Serial.print(kpd.key[i].kchar);
          Serial.println(msg);
        }
      }
    }

  MidiUSB.flush();
  FastLED.show();


}

void start_leds() {

  int change_red = 0;
  int change_green = 0;
  int change_blue = 0;

  int diffrence_in_color = 30;
  for(int i = 0; i < NUM_LEDS; i++) {

    leds[i] = CRGB (
      change_red,
      change_green,
      change_blue
      );
      
    change_red += diffrence_in_color;
    change_green += diffrence_in_color;
    change_blue += diffrence_in_color;
    FastLED.show();
    delay(100);
  }
  
}

void set_color_ametysth() {
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Amethyst;
  }
}

void set_color_black() {
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
}

void set_color_random() {
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB (
      rand() %(255 - 0 + 1) + 0,
      rand() %(255 - 0 + 1) + 0, 
      rand() %(255 - 0 + 1) + 0
      );
  }
}

void set_color_magenta() {
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Magenta;
  }
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
      // red = rand() %(255 - 0 + 1);
      // green = rand() %(255 - 0 + 1);
      // blue = rand() %(255 - 0 + 1);
      set_color_random();
      return 0;
      break;
  }

  for(int i = 0; i < num_of_leds_on; i++) {
    leds[i].setRGB(red, green, blue);
  } 
  FastLED.show();
}



