#define ESP32DEBUGGING
#include <ESP32Logger.h>

#include "bubble_led_clock.h"
#include "app_pref.h"
#include <Arduino.h>
#include <Preferences.h>
#include <string.h>

#define APP_PREF_WIFI_SSID "wifi_ssid"
#define APP_PREF_PASSWORD "password"
#define APP_PREF_TIME_ZONE "time_zone"
#define APP_PREF_LOG_LEVEL "log_level"

AppPreferences appPrefs{};

void AppPreferences::setup() {
  prefs.begin("config");
  getPreferences();
}

void AppPreferences::getPreferences() {
  prefs.getString(APP_PREF_WIFI_SSID, config.ssid, sizeof(config.ssid));
  prefs.getString(APP_PREF_PASSWORD, config.password, sizeof(config.password));
  prefs.getString(APP_PREF_TIME_ZONE, config.time_zone,
                  sizeof(config.time_zone));

  config.logLevel =
      prefs.getInt(APP_PREF_LOG_LEVEL, int32_t(ESP32LogLevel::Info)); // debug level Info
  if (config.logLevel < int32_t(ESP32LogLevel::Error))
    config.logLevel = int32_t(ESP32LogLevel::Error);
  else if (config.logLevel > int32_t(ESP32LogLevel::Debug))
    config.logLevel = int32_t(ESP32LogLevel::Debug);

  setenv("TZ", config.time_zone, 1);
  tzset();
}

void AppPreferences::putPreferences() {
  prefs.putString(APP_PREF_WIFI_SSID, config.ssid);
  prefs.putString(APP_PREF_PASSWORD, config.password);
  prefs.putString(APP_PREF_TIME_ZONE, config.time_zone);
  prefs.putInt(APP_PREF_LOG_LEVEL, config.logLevel);
}

void AppPreferences::dumpPreferences() {
  DBGLOG(Debug, "%s: %s", APP_PREF_WIFI_SSID, config.ssid);
  DBGLOG(Debug, "%s: %s", APP_PREF_PASSWORD, "***");
  DBGLOG(Debug, "%s: %s", APP_PREF_TIME_ZONE, config.time_zone);
  DBGLOG(Debug, "%s: %d", APP_PREF_LOG_LEVEL, config.logLevel);
}
