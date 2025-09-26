#ifndef BLC_TYPES_H
#define BLC_TYPES_H

#include <Arduino.h>

// --- Shared Animation & Scene Types ---
enum AnimationType {
  SLOT_MACHINE,
  MATRIX,
  SCROLLING
};

struct DisplayScene {
  const char* scene_name;
  const char* format_string;
  AnimationType animation_type;
  bool dots_with_previous;
  unsigned long duration_ms;
  unsigned long anim_param_1;
  unsigned long anim_param_2;
  float (*getDataValue)(); 
};

// --- Shared Access Point Form Types ---
// A generic struct to hold the text and value for a dropdown option
struct PrefSelectOption {
  const char* name;  // The user-friendly text (e.g., "Eastern Time")
  const char* value; // The value to save (e.g., "EST5EDT,M3.2.0...")
};

enum AppLogLevel {
  APP_LOG_ERROR,
  APP_LOG_INFO,
  APP_LOG_DEBUG
};

enum FormFieldIndex {
  WIFI_SSID, PASSWORD, TIME_ZONE, LOG_LEVEL, SHOW_STARTUP_ANIM, OWM_CITY,
  OWM_STATE_CODE, OWM_COUNTRY_CODE, OWM_API_KEY, TEMP_UNITS,
  NUM_FORM_FIELDS
};

enum PrefType { 
  PREF_NONE, PREF_STRING, PREF_BOOL, PREF_INT, PREF_ENUM, PREF_SELECT
};

enum FieldValidation {
  VALIDATION_NONE, VALIDATION_IP_ADDRESS, VALIDATION_INTEGER,
};

typedef struct {
  const char *id;
  const char *name;
  bool isMasked;
  FieldValidation validation;
  PrefType prefType;
  union {
    char *str_pref;
    bool *bool_pref;
    int32_t *int_pref;
    AppLogLevel *enum_pref;
  } pref;
  const PrefSelectOption* select_options;
  int num_select_options;
} FormField;

#endif // BLC_TYPES_H
