#include "leds.h"



leds::leds() {
    
    leds_palette = RainbowStripeColors_p;
    leds_blend = LINEARBLEND;
    leds_arr[NUM_LEDS];
    updates_per_second = 100;
    leds_index = 0;
    brightness = 255;
    leds_timer = 0;
}
    

void leds::initialize_leds() {

    FastLED.addLeds<LED_TYPE, LED_PIN, GRB>(leds_arr, NUM_LEDS);
    start_leds();
}

void leds::start_leds() {

    unsigned long starting_time = millis();
    do {

        fill_leds_palette();
        FastLED.show();
        delay(10);
        // Serial.println(leds_index);
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
    leds_index++;
}

void leds::set_leds_palette(uint8_t mode) {

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
    case 3:
    leds_palette = LavaColors_p;
      break;
  };
}

void leds::set_leds_blend(TBlendType _leds_blend) {

    leds_blend = _leds_blend;
}

void leds::update_leds() {

    if(millis() - leds_timer > (1000 / updates_per_second)) {

        leds_timer = millis();
        fill_leds_palette();
        FastLED.show();
    }
    
}