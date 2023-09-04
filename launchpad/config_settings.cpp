#include "config_settings.h"


config_settings::config_settings() {}   
config_settings::config_settings(scale _scale, uint8_t _mode, lau_midi _lau_midi) {

    settings_scale = _scale;
    mode = _mode;
    settings_lau_midi = _lau_midi;

}
void config_settings::store_settings(scale _scale, uint8_t _mode, lau_midi _lau_midi ) {
    
    if( settings_scale.get_tonic() == _scale.get_tonic() && 
        settings_scale.get_tonality() == _scale.get_tonality() && 
        settings_scale.get_chord_notes() == _scale.get_chord_notes() && 
        mode == _mode && 
        settings_lau_midi.base_velocity == _lau_midi.base_velocity && 
        settings_lau_midi.base_channel == _lau_midi.base_channel) return;

    Serial.println("Storing settings to EEPROM!!!");
    // StaticJsonDocument<96> doc;

    // doc["tonic"] = _scale.get_tonic();
    // doc["tonality"] = _scale.get_tonality();
    // doc["chord_notes"] = _scale.get_chord_notes();
    // doc["mode"] = _mode;
    // doc["base_velocity"] = _base_velocity;
    // doc["base_channel"] = _base_channel;

    // EepromStream eepromStream(0, doc.size());
    // serializeJson(doc, eepromStream);
    // serializeJson(doc, Serial);
    //load_from_EEPROM();
}


void config_settings::load_from_EEPROM() {
    
    Serial.println("Loading settings from EEPROM!!!");
    // StaticJsonDocument<96> doc;

    // EepromStream eepromStream(0, doc.size());
    // deserializeJson(doc, eepromStream);

    // settings_scale.set_tonic(doc["tonic"]);
    // settings_scale.set_tonality(doc["tonality"]);
    // settings_scale.set_chord_notes(doc["chord_notes"]);
    // mode = doc["mode"];
    // settings_lau_midi.base_velocity = doc["base_velocity"];
    // settings_lau_midi.base_channel = doc["base_channel"];

}


