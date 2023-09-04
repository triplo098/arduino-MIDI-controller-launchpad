#include "leds.h"

leds::leds() {
    
    leds_palette = RainbowStripeColors_p;
    leds_blend = LINEARBLEND;
    leds_arr[NUM_LEDS];
    updates_per_second = 100;
    brightness = 255;
}   

void leds::initialize_leds() {

    FastLED.addLeds<LED_TYPE, LED_PIN, GRB>(leds_arr, NUM_LEDS);
    FastLED.setBrightness(brightness);
    start_leds();
}

void leds::start_leds() {

    unsigned long starting_time = millis();
    do {
        update_leds(START_MODE);
    }
    while(millis() - starting_time < 2000);
}

void leds::fill_leds_palette(uint8_t start_index) {

    for( int i = 0; i < NUM_LEDS; ++i) {

        leds_arr[i] = ColorFromPalette(
            leds_palette, start_index, 
            brightness, leds_blend
        );
        start_index += 2;
    }
}

void leds::update_leds(uint8_t _mode) {

    static uint8_t leds_index = 0;
    static unsigned long leds_timer = millis();

    switch(_mode) {
    case SETUP_MODE: default:
    leds_palette = ForestColors_p;
      break;
    case NORMAL_MODE:
    leds_palette = CloudColors_p;
      break;
    case AUTO_MODE:
    leds_palette = OceanColors_p;
      break;
    case NOTE_ON_MODE:
    leds_palette = LavaColors_p;
      break;
    case START_MODE:
    leds_palette = RainbowColors_p;
      break;
    };

    if(millis() - leds_timer > (1000 / updates_per_second)) {

        leds_timer = millis();
        fill_leds_palette(leds_index);
        FastLED.show();
        leds_index++;
    }
}