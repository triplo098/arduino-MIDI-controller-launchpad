#ifndef LEDS_H
#define LEDS_H

#include "FastLED.h"

class leds : public FastLED {

    private:
        TProgmemRGBPalette16 leds_palette;
        TBlendType leds_blend;
        CRGB leds_arr[];
        int updates_per_second;
        uint16_t leds_index;
        uint16_t brightness;

        unsigned long leds_timer;

    public:
        void initialize_leds();
        void start_leds();
        void fill_leds_palette();
        void set_leds_palette();
        void set_leds_blend();
            
}


#endif