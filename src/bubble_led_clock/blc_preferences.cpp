#include "debug.h"
#include "blc_preferences.h"

#include <ESP32NTPClock.h>

#include <Preferences.h>
#include <Arduino.h>
#include <string.h>

#define APP_PREF_WIFI_SSID "wifi_ssid"
#define APP_PREF_PASSWORD "password"
#define APP_PREF_TIME_ZONE "time_zone"
#define APP_PREF_LOG_LEVEL "log_level"
#define APP_PREF_SHOW_STARTUP_ANIM "startup_anim"
#define APP_PREF_OWM_API_KEY "owm_api_key"
#define APP_PREF_OWM_CITY    "owm_city"
#define APP_PREF_OWM_STATE_CODE "owm_state"
#define APP_PREF_OWM_COUNTRY_CODE "owm_country"
#define APP_PREF_TEMP_UNIT "temp_unit"

#define PREF_NAMESPACE "config"

void AppPreferences::getPreferences() {

  BasePreferences::getPreferences();

  prefs.begin(PREF_NAMESPACE, true); // Open read-only

  config.showStartupAnimation = prefs.getBool(APP_PREF_SHOW_STARTUP_ANIM, true); 

  prefs.getString(APP_PREF_OWM_API_KEY, config.owm_api_key, sizeof(config.owm_api_key));

  if (prefs.isKey(APP_PREF_OWM_CITY)) {
    prefs.getString(APP_PREF_OWM_CITY, config.owm_city, sizeof(config.owm_city));
  } else {
    strncpy(config.owm_city, "New York", sizeof(config.owm_city));
  }  
  
  if (prefs.isKey(APP_PREF_OWM_STATE_CODE)) {
    prefs.getString(APP_PREF_OWM_STATE_CODE, config.owm_state_code, sizeof(config.owm_state_code));
  } else {
    strncpy(config.owm_state_code, "NY", sizeof(config.owm_state_code));
  }
  
  if (prefs.isKey(APP_PREF_OWM_COUNTRY_CODE)) {
    prefs.getString(APP_PREF_OWM_COUNTRY_CODE, config.owm_country_code, sizeof(config.owm_country_code));
  } else {
    strncpy(config.owm_country_code, "US", sizeof(config.owm_country_code));
  }
  
  if (prefs.isKey(APP_PREF_TEMP_UNIT)) {
    prefs.getString(APP_PREF_TEMP_UNIT, config.tempUnit, sizeof(config.tempUnit));
  } else {
    strncpy(config.tempUnit, "imperial", sizeof(config.tempUnit));
  }  
 
  prefs.end();
}

void AppPreferences::putPreferences() {

  BasePreferences::putPreferences(); 

  prefs.begin(PREF_NAMESPACE, false); // Open read-write
  prefs.putBool(APP_PREF_SHOW_STARTUP_ANIM, config.showStartupAnimation);
  prefs.putString(APP_PREF_OWM_API_KEY, config.owm_api_key);
  prefs.putString(APP_PREF_OWM_CITY, config.owm_city);
  prefs.putString(APP_PREF_OWM_STATE_CODE, config.owm_state_code);
  prefs.putString(APP_PREF_OWM_COUNTRY_CODE, config.owm_country_code);
  prefs.putString(APP_PREF_TEMP_UNIT, config.tempUnit);
  prefs.end(); // close the connection to the storage namespace 
}

void AppPreferences::dumpPreferences() {

  BasePreferences::dumpPreferences(); 

  LOGMSG(APP_LOG_DEBUG, "Pref=%s: %s", APP_PREF_SHOW_STARTUP_ANIM, config.showStartupAnimation ? "Yes" : "No");
  LOGMSG(APP_LOG_DEBUG, "Pref=%s: %s", APP_PREF_OWM_CITY, config.owm_city);
  LOGMSG(APP_LOG_DEBUG, "Pref=%s: %s", APP_PREF_OWM_STATE_CODE, config.owm_state_code);
  LOGMSG(APP_LOG_DEBUG, "Pref=%s: %s", APP_PREF_OWM_COUNTRY_CODE, config.owm_country_code);
  LOGMSG(APP_LOG_DEBUG, "Pref=%s: %s", APP_PREF_OWM_API_KEY, "***"); 
  LOGMSG(APP_LOG_DEBUG, "Pref=%s: %s", APP_PREF_TEMP_UNIT, config.tempUnit);
}
