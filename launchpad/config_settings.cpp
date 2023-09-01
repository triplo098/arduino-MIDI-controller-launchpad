#include "config_settings.h"


config_settings::config_settings(scale _scale, uint8_t _mode, uint8_t _base_velocity, uint8_t _base_channel) {

    scale = _scale;
    mode = _mode;
    base_velocity = _base_velocity;
    base_channel = _base_channel;

}

void config_settings::store_settings(scale _scale, uint8_t _mode, uint8_t _base_velocity, uint8_t _base_channel) {
    
    StaticJsonDocument<96> doc;

    doc["tonic"] = scale.get_tonic();
    doc["tonality"] = scale.get_tonality();
    doc["chord_notes"] = scale.get_chord_notes();
    doc["mode"] = mode;
    doc["base_velocity"] = base_velocity;
    doc["base_channel"] = base_channel;

    // EepromStream eepromStream(0, doc.size());
    // serializeJson(doc, eepromStream);
    serializeJson(doc, Serial);
    
}

void config_settings::load_settings() {

    StaticJsonDocument<96> doc;

    EepromStream eepromStream(0, doc.size());
    deserializeJson(doc, eepromStream);

    scale.set_tonic(doc["tonic"]);
    scale.set_tonality(doc["tonality"]);
    scale.set_chord_notes(doc["chord_notes"]);
    mode = doc["mode"];
    base_velocity = doc["base_velocity"];
    base_channel = doc["base_channel"];

}


