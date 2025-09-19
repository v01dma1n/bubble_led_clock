#ifndef BLC_PREFERENCES_H
#define BLC_PREFERENCES_H

#include "blc_access_point.h"

#include <Arduino.h>
#include <IPAddress.h>
#include <Preferences.h>

#define MAX_PREF_STRING_LEN 64

#define OWM_UNIT_IMPERIAL "imperial"
#define OWM_UNIT_METRIC "metric"

struct AppConfig {
  char ssid[MAX_PREF_STRING_LEN];
  char password[MAX_PREF_STRING_LEN];
  char time_zone[MAX_PREF_STRING_LEN];
  AppLogLevel logLevel;
  char owm_api_key[MAX_PREF_STRING_LEN];
  char owm_city[MAX_PREF_STRING_LEN];
  char owm_state_code[MAX_PREF_STRING_LEN];
  char owm_country_code[MAX_PREF_STRING_LEN];
  char tempUnit[MAX_PREF_STRING_LEN];
};

class AppPreferences {
protected:
  Preferences prefs;

public:
  AppConfig config;

  void setup();
  void getPreferences();
  void putPreferences();
  void dumpPreferences();
};

// extern AppPreferences appPrefs;

#endif // BLC_PREFERENCES_H
