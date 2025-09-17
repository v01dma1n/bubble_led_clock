#include "blc_app.h"

// Define the mutex here
SemaphoreHandle_t serialMutex = NULL;

// The single global instance of the application
BubbleLedClockApp& app = BubbleLedClockApp::getInstance();

void setup() {
  
  Serial.begin(115200);
  delay(200);
  Serial.println("\nStarting...");

  // Create the mutex before any other tasks might use it
  serialMutex = xSemaphoreCreateMutex();

  app.setup();

  app.getPrefs().dumpPreferences();

  snprintf(app.getPrefs().config.owm_api_key, MAX_PREF_STRING_LEN, "6639678a4c920f9349923ff9467d7747");
  app.getPrefs().putPreferences();

  Serial.println("\n... setup complete");
}

void loop() {
  app.loop();
}