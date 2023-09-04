#ifndef CONFIG_SETTINGS_H
#define CONFIG_SETTINGS_H

#include <StreamUtils.h>
#include "ArduinoJson.h"
#include "scale.h"
#include "lau_midi.h"
#include "EEPROM.h"

#define LED_PIN 10
#define NUM_LEDS 15
#define LED_TYPE WS2812B
#define ACCEPTANCE_RATE 32

#define SETUP_MODE 0
#define NORMAL_MODE 1
#define AUTO_MODE 2 
#define NOTE_ON_MODE 3
#define START_MODE 4

class config_settings {

	public:
		config_settings();
		config_settings(scale _scale, uint8_t _mode, lau_midi _lau_midi);
		void store_settings(scale _scale, uint8_t _mode, lau_midi _lau_midi);
		void load_from_EEPROM();
		uint8_t mode;
	private:	
		lau_midi settings_lau_midi;	
		scale settings_scale;

};

#endif