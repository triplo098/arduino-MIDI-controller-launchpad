#ifndef CONFIG_SETTINGS_H
#define CONFIG_SETTINGS_H

#include <StreamUtils.h>

class config_settings {

	static public:
		void store_settings();
		void change_mode(uint64_t mode);
	static private:	
		uint8_t mode;
		uint8_t base_velocity;
		uint8_t base_channel;

}

#endif