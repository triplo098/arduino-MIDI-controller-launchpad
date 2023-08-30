#include "lan_keypad.h"


#define ROWS 4
#define COLS 4

lan_keypad::lan_keypad() {

    keys[ROWS][COLS];
    rowPins[ROWS] = {5, 4, 3, 2};
    colPins[COLS] = {9, 8, 7, 6}; 
    keys_hold_timer = 0;
    change_mode_count = 0;

}

void lan_keypad::initialize_keypad(scale::scale scale) {

    this.Keypad::Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

    notes_to_keypad(scale);
}

void lan_keypad::notes_to_keypad(scale::scale scale) {

    uint8_t note_index = 0;
    uint8_t octave = 0;

    for(int i = 0; i < ROWS; i++) {
        for(int j = 0; j < COLS; j++) {

        acc_note += scale.notes[note_index] + (12*octave);

        keys[k][n] = acc_note;

        note_index++;
        if(note_index == scale.count_notes()) 
        
            octave++;
            note_index %= scale.count_notes();
        }
    }
}

void lan_keypad::activate_keypad() {

  /*********************
  Modified part of code from Keypad/examples/MultiKey/MultiKey.ino
  *********************/
  if (this.getKeys()) {
    
    for (int i = 0; i < LIST_MAX; i++) {
       
      if ( this.key[i].stateChanged ) {

        uint8_t note = 0;
        switch (this.key[i].kstate) {  

          // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
          case PRESSED:

            if(this.key[i].kchar == keys[3][3]) change_mode_count++;
            else if(this.key[i].kchar == keys[3][0]) change_mode_count++;

            if(change_mode_count == 2) {
              keys_hold_timer = millis();
              continue;
            } 
                
            note = (uint8_t) this.key[i].kchar;

            send_notes(true, note);
        
            break;
          case HOLD:
            break;
          case RELEASED:

            if(this.key[i].kchar == keys[3][3] 
            || this.key[i].kchar == keys[3][0]) change_mode_count = 0;

            if(change_mode_count < 2) keys_hold_time = 0;
            
            note = (uint8_t) this.key[i].kchar;
            
            send_notes(false, note);

            break;
          case IDLE:
            break;
        }
      }      
    }
  }

  if((millis() - keys_hold_time > 1000) && keys_hold_timer != 0) {
    
    config_settings.mode = (congif_settings.mode + 1) % 3;
    keys_hold_timer = 0;
  }
}

void send_notes(bool key_active, uint8_t note) {

    
    Serial.print("Note: ");
    Serial.println(note);

}