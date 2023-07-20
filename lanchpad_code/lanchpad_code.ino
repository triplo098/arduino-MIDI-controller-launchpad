/*
  pot = potentiometers

*/
#include "MIDIUSB.h"
#include <Keypad.h>
#include <FastLED.h>

#define LED_PIN     14
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

byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2}; 

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );


String msg;

//
//potentiometers
//
const byte number_of_pot = 4;

struct potentiometer {

  int pin;
  int control_number;
  int pre_value;
  int value;
  int channel = 0;

};

struct potentiometer potentiometers[number_of_pot];


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

  potentiometers[0].pin = A0;
  potentiometers[1].pin = A1;
  potentiometers[2].pin = A2;
  potentiometers[3].pin = A3;

  potentiometers[0].control_number = 1;   //modulation
  potentiometers[1].control_number = 7;   //volume
  potentiometers[2].control_number = 10;  //pan
  potentiometers[3].control_number = 74;  //brigthness

  Serial.begin(9600);
  msg = "";

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS); 
  FastLED.setBrightness(100);
}

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

int pin;
int pre_value;
int value;
int control_number;
int channel;

void loop() {


  
  //controloing potentiometers
  for(int i = 0; i < number_of_pot; i++) {
    
    pin = potentiometers[i].pin;
    potentiometers[i].value = analogRead(pin) / 8;
    
    channel = potentiometers[i].channel;
    control_number = potentiometers[i].control_number;
    value = potentiometers[i].value;


    if(potentiometers[i].pre_value == potentiometers[i].value) {
      continue;
    }
    controlChange(channel, control_number, value);
    
    potentiometers[i].pre_value = potentiometers[i].value;
    
    
    Serial.println(value);

    if( i == 3) {
      FastLED.setBrightness(potentiometers[i].value * 2);
    }
  }
  
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

              note = kpd.key[i].kchar;
              noteOn(0, note, 127);
              Serial.print("Note On");
              Serial.println(note);

              set_color_random();
              //set_color_ametysth();
             
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
              set_color_black();

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



