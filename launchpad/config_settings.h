#ifndef CONFIG_SETTINGS_H
#define CONFIG_SETTINGS_H

#include <StreamUtils.h>
#include "ArduinoJson.h"
#include "scale.h"
#include "EEPROM.h"

#define LED_PIN 10
#define NUM_LEDS 15
#define LED_TYPE WS2812B
#define ACCEPTANCE_RATE 32

#define SETUP_MODE 0
#define NORMAL_MODE 1
#define AUTO_MODE 2 
#define KEY_ON_MODE 3 




class config_settings {

	public:
		config_settings();
		config_settings(scale _scale, uint8_t _mode, uint8_t _base_velocity, uint8_t _base_channel);
		void store_settings(scale _scale, uint8_t _mode, uint8_t _base_velocity, uint8_t _base_channel);
		void load_from_EEPROM();
		uint8_t mode;
	private:	
		uint8_t base_velocity;
		uint8_t base_channel;
		scale settings_scale;
		

};

#endif