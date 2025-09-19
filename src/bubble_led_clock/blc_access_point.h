#ifndef BLC_ACCESS_POINT_H
#define BLC_ACCESS_POINT_H

#include "blc_types.h"

#include <Arduino.h>
#include <cstdint>

extern volatile bool g_isClientConnected;

#define DNS_SERVER_PORT 53

enum FormFieldIndex {
  WIFI_SSID,
  PASSWORD,
  TIME_ZONE,
  LOG_LEVEL,
  OWM_CITY,
  OWM_STATE_CODE,
  OWM_COUNTRY_CODE,
  OWM_API_KEY,
  TEMP_UNITS,
  NUM_FORM_FIELDS
};

enum PrefType { 
  PREF_NONE, 
  PREF_STRING, 
  PREF_BOOL, 
  PREF_INT, 
  PREF_ENUM,
  PREF_SELECT
};

enum FieldValidation {
  VALIDATION_NONE,
  VALIDATION_IP_ADDRESS,
  VALIDATION_INTEGER,
  // VALIDATION_ENUM
};

enum AppLogLevel {
  APP_LOG_ERROR, // 0
  APP_LOG_INFO,  // 1
  APP_LOG_DEBUG  // 3
};

typedef struct {
  const char *id;             // HTML element ID
  const char *name;           // HTML element name (optional)
  bool isMasked;              // Password needs to be masked
  FieldValidation validation; // what kind of validation needs to be performed
  PrefType prefType;
  union {
    char *str_pref;
    bool *bool_pref;
    int32_t *int_pref;
    AppLogLevel *enum_pref;
  } pref;
  String value;  // Current value
  bool received; // Flag indicating if a value was received

  const PrefSelectOption* select_options; // Pointer to an array of options
  int num_select_options;                 // The number of options in the array

  void setValue(const String &newValue) { value = newValue; }
} FormField;

void processAPInput();
void setupAP(const char *hostName);
void loopAP(unsigned long apRebootTimeMillis);

#endif // ACCESS_POINT
