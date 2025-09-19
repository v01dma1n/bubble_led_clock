#include "debug.h"
#include "blc_preferences.h"

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

void AppPreferences::setup() {
  prefs.begin(PREF_NAMESPACE);
  getPreferences();
}

void AppPreferences::getPreferences() {
  prefs.getString(APP_PREF_WIFI_SSID, config.ssid, sizeof(config.ssid));

  prefs.getString(APP_PREF_PASSWORD, config.password, sizeof(config.password));

  if (prefs.isKey(APP_PREF_TIME_ZONE)) {
    prefs.getString(APP_PREF_TIME_ZONE, config.time_zone, sizeof(config.time_zone));
  } else {
    strncpy(config.time_zone, "EST5EDT,M3.2.0,M11.1.0", sizeof(config.time_zone));
  }

  int32_t savedLogLevel = prefs.getInt(APP_PREF_LOG_LEVEL, APP_LOG_INFO);
  config.logLevel = static_cast<AppLogLevel>(savedLogLevel);
  if (config.logLevel < APP_LOG_ERROR)
    config.logLevel = APP_LOG_ERROR;
  else if (config.logLevel > APP_LOG_DEBUG)
    config.logLevel = APP_LOG_DEBUG;

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
 
}

void AppPreferences::putPreferences() {
  prefs.clear(); // Clear the entire namespace first to ensure a clean write 
  prefs.putString(APP_PREF_WIFI_SSID, config.ssid);
  prefs.putString(APP_PREF_PASSWORD, config.password);
  prefs.putString(APP_PREF_TIME_ZONE, config.time_zone);
  prefs.putInt(APP_PREF_LOG_LEVEL, config.logLevel);
  prefs.putBool(APP_PREF_SHOW_STARTUP_ANIM, config.showStartupAnimation);
  prefs.putString(APP_PREF_OWM_API_KEY, config.owm_api_key);
  prefs.putString(APP_PREF_OWM_CITY, config.owm_city);
  prefs.putString(APP_PREF_OWM_STATE_CODE, config.owm_state_code);
  prefs.putString(APP_PREF_OWM_COUNTRY_CODE, config.owm_country_code);
  prefs.putString(APP_PREF_TEMP_UNIT, config.tempUnit);
  prefs.end(); // close the connection to the storage namespace 
}

void AppPreferences::dumpPreferences() {
  LOGMSG(APP_LOG_DEBUG, "Pref=%s: %s", APP_PREF_WIFI_SSID, config.ssid);
  LOGMSG(APP_LOG_DEBUG, "Pref=%s: %s", APP_PREF_PASSWORD, "***");
  LOGMSG(APP_LOG_DEBUG, "Pref=%s: %s", APP_PREF_TIME_ZONE, config.time_zone);
  LOGMSG(APP_LOG_DEBUG, "Pref=%s: %d", APP_PREF_LOG_LEVEL, config.logLevel);
  LOGMSG(APP_LOG_DEBUG, "Pref=%s: %s", APP_PREF_SHOW_STARTUP_ANIM, config.showStartupAnimation ? "Yes" : "No");
  LOGMSG(APP_LOG_DEBUG, "Pref=%s: %s", APP_PREF_OWM_CITY, config.owm_city);
  LOGMSG(APP_LOG_DEBUG, "Pref=%s: %s", APP_PREF_OWM_STATE_CODE, config.owm_state_code);
  LOGMSG(APP_LOG_DEBUG, "Pref=%s: %s", APP_PREF_OWM_COUNTRY_CODE, config.owm_country_code);
  LOGMSG(APP_LOG_DEBUG, "Pref=%s: %s", APP_PREF_OWM_API_KEY, "***"); 
  LOGMSG(APP_LOG_DEBUG, "Pref=%s: %s", APP_PREF_TEMP_UNIT, config.tempUnit);
}
