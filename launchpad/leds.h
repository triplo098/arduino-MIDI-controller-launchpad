#ifndef LEDS_H
#define LEDS_H

#include "FastLED.h"
#include "Arduino.h"
#include "config_settings.h"


class leds {

    private:
      leds();
      CRGBPalette16 leds_palette;
      TBlendType leds_blend;
      CRGB leds_arr[NUM_LEDS];
      uint8_t leds_index;
      uint8_t brightness;
      unsigned long leds_timer;

      void start_leds();
      void set_color_and_progres(int pot_index, double progress);

    public:
        int updates_per_second;
        void initialize_leds();
        void fill_leds_palette();
        void set_leds_palette(uint8_t mode);
        void set_leds_blend(TBlendType _leds_blend);
        void update_leds();

};


#endif