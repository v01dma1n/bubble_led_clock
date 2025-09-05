#ifndef APP_PREF_H
#define APP_PREF_H

#define ESP32DEBUGGING
#include <Arduino.h>
#include <IPAddress.h>
#include <Preferences.h>

#define PING_IP_COUNT 4
#define MAX_PREF_STRING_LEN 64

struct AppConfig {
  char ssid[MAX_PREF_STRING_LEN];
  char password[MAX_PREF_STRING_LEN];
  char time_zone[MAX_PREF_STRING_LEN];
  int32_t logLevel;
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

extern AppPreferences appPrefs;

#endif // APP_PREF_H
