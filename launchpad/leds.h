#ifndef LEDS_H
#define LEDS_H

#include "FastLED.h"
#include "Arduino.h"
#include "config_settings.h"

class leds {

    private:
      CRGBPalette16 leds_palette;
      TBlendType leds_blend;
      CRGB leds_arr[NUM_LEDS];
      uint8_t brightness;

      void start_leds();
      void set_color_and_progres(int pot_index, double progress);
      
    public:
        int updates_per_second;
        leds();
        void initialize_leds();
        void fill_leds_palette(uint8_t start_index);
        void update_leds(uint8_t _mode);

};

#endif