#ifndef ACCESS_POINT_H
#define ACCESS_POINT_H

#define DNS_SERVER_PORT 53

enum FormFieldIndex {
  WIFI_SSID,
  PASSWORD,
  TIME_ZONE,
  LOG_LEVEL,
  NUM_FORM_FIELDS
};

enum PrefType { 
  PREF_NONE, 
  PREF_STRING, 
  PREF_BOOL, 
  PREF_INT, 
  PREF_ENUM
};

enum FieldValidation {
  VALIDATION_NONE,
  VALIDATION_IP_ADDRESS,
  VALIDATION_INTEGER,
  // VALIDATION_ENUM
};

enum LogLevel {
  ERROR, // 0
  INFO,  // 1
  DEBUG  // 3
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
    LogLevel *enum_pref;
  } pref;
  String value;  // Current value
  bool received; // Flag indicating if a value was received

  void setValue(const String &newValue) { value = newValue; }
} FormField;

void processAPInput();
void setupAP(const char *hostName);
void loopAP(unsigned long apRebootTimeMillis);

#endif // ACCESS_POINT
