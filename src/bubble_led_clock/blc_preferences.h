#ifndef BLC_PREFERENCES_H
#define BLC_PREFERENCES_H

#include "blc_types.h"
#include <base_preferences.h> // <-- Include the new library header

// AppConfig now inherits from BaseConfig and only adds specific fields
struct AppConfig : public BaseConfig {
  bool showStartupAnimation;
  char owm_api_key[MAX_PREF_STRING_LEN];
  char owm_city[MAX_PREF_STRING_LEN];
  char owm_state_code[MAX_PREF_STRING_LEN];
  char owm_country_code[MAX_PREF_STRING_LEN];
  char tempUnit[MAX_PREF_STRING_LEN];
};

// AppPreferences inherits from BasePreferences
class AppPreferences : public BasePreferences {
public:
    // The constructor now passes its own config object to the base class
    AppPreferences() : BasePreferences(config) {}

    // We will override these to add our specific logic
    void getPreferences() override;
    void putPreferences() override;
    void dumpPreferences() override;

    // The application owns the full AppConfig struct
    AppConfig config;
};

#endif // BLC_PREFERENCES_H