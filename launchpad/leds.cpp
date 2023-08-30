#include "leds.h"
#include "Arduino.h"


#define LED_PIN 10
#define NUM_LEDS 15
#define LED_TYPE WS2812B

leds::leds() {
    
    leds_palette = RainbowColors_p;
    leds_blend = LINEARBLEND;
    leds_arr[NUM_LEDS];
    updates_per_second = 100;
    leds_index = 0;
    brightness = 255;
    leds_timer = 0;
}
    

void leds::initialize_leds() {

    this.addLeds<LED_TYPE, LED_PIN, GRB>(leds_arr, NUM_LEDS);
}

void leds::start_leds() {

    unsigned long starting_time = millis();
    do {

        fill_leds_palette(leds_start_index);
        FastLED.show();
        delay(10);
        // Serial.println(leds_start_index);
        leds_index++;
    }
    while(millis() - starting_time < 2000);
}

void leds::fill_leds_palette() {

    for( int i = 0; i < NUM_LEDS; ++i) {

        leds_arr[i] = ColorFromPalette(
            leds_palette, leds_index, 
            brightness, leds_blend
        );

        leds_index += 2;
    }
}

void leds::set_leds_palette(TProgmemRGBPalette16 leds_palette) {

    this.leds_palette = leds_palette;
}

void leds::set_leds_blend(TBlendType leds_blend) {

    this.leds_blend = leds_blend;
}