#ifndef DEBUG_H
#define DEBUG_H

#include "Arduino.h"
#include "blc_app.h" // Needed to access the preferences singleton

extern SemaphoreHandle_t serialMutex;

#define LOGMSG(level, format, ...) \
    do { \
        /* Only print if the configured level is high enough for this message */ \
        if (BubbleLedClockApp::getInstance().getPrefs().config.logLevel >= level) { \
            if (serialMutex != NULL && xSemaphoreTake(serialMutex, portMAX_DELAY) == pdTRUE) { \
                Serial.printf(format "\n", ##__VA_ARGS__); \
                xSemaphoreGive(serialMutex); \
            } \
        } \
    } while (0)

#define LOGERR(format, ...) LOGMSG(APP_LOG_ERROR, format, ##__VA_ARGS__)
#define LOGINF(format, ...)  LOGMSG(APP_LOG_INFO,  format, ##__VA_ARGS__)
#define LOGDBG(format, ...) LOGMSG(APP_LOG_DEBUG, format, ##__VA_ARGS__)

#endif // DEBUG_H