#ifndef VERSION_H
#define VESTION_H

#define APP_NAME "Bubble LED Clock"
#define APP_AUTHOR "v01dma1n"
#define APP_DATE "2025-09-19"

#define VER_MAJOR 1
#define VER_MINOR 8
#define VER_BUILD 0

#define APP_MESSAGE "The clock connects to WiFi. Double-press Reset for Access Point."

#include <string>

#define VERSION_STRING \
  (std::to_string(VER_MAJOR) + "." + std::to_string(VER_MINOR) + "." + std::to_string(VER_BUILD))

/* Known Problems

  #issue002 Temperature and humidity is displayed as truncated NO DATA when OpenWeather is not configured.
  Perhaps we should not display anything if the token is undefined.
  
  #issue004 Debug enum validation is hardcoded in access_point.c
   form += "  <option value=\"" + String(APP_LOG_ERROR) + "\">Error</option>\n";
   form += "  <option value=\"" + String(APP_LOG_INFO) + "\">Info</option>\n";
   form += "  <option value=\"" + String(APP_LOG_DEBUG) + "\">Debug</option>\n";

  #issue008 OpenWeather API client is broken
  https://api.openweathermap.org/data/2.5/weather?q=,NJ,US&appid=<removed>&units=imperial
  16:29:08.084 -> HTTP GET request failed, error: 

  #issue009 The double-reset interval is too long
  The Double reset press must be separated at least by 1 second.
  This is too long.

  #issue010 Temperature and Humidity NO DATA should not be displayed

  #issue011 The clock should also work without WiFi
  The configuration should allow disabling WiFi and setting the clock with AP

  #issue012 The messages on the display are not clear 
  No explicit information that the WiFi is not available or not configured
  Add AP message after client connects: Navigate to 192.1.4.1

  #issue013 The logic should not call OpenWeather when there is no Internet

*/

/* Version history

2025-09-18 v.1.8.0
-  The time of displaying other than time value is half of the time displaying
-  Resolved: #issue005 The time is incorrect when clock starts withoug WiFi.

2025-09-18 v.1.7.1 
- dots with previous character added to the scrolling animation
- enable/disable startup message (note: preference keys must be less then or equal 15 chars)
- boolean config in captive portal fixed for unchecked checkboxes

2025-09-18 v.1.6 OpenWeather Access Restored
- fixed mismatch between non-secure client and https protocol in URL
- Temperature in F or C selection in the Preferences
- AP activation on double-reset press fixed

2025-09-18 v.1.5 Time is displayed now between temperature and humidity
- added time scene between temperature and humidity
- changed the date animation to matrix
- removed Adafruit Backpack library
- double-reset for AP is now time based
- the preference saving now is forced by .end() method all

2025-09-17 v.1.4 Access Point on Boot Press
- #issue001 No way to start AP if WiFi is accessible
- #issue007 The app name should not be hardcoded "AP MODE -- CONNECT TO BUBBLE-CLOCK"
  auto apMessage = std::make_unique<ScrollingTextAnimation>("AP MODE -- CONNECT TO BUBBLE-CLOCK");

2025-09-15 v.1.3 Code Refactoring 
- Clock class renamed to DisplayManager
- LOGMSG with log level added

2025-09-15 v.1.2 Code Refactoring and file renaminig
- The units are renamed with blc_ prefix if project-specific
- Class names are aligned with units

2025-09-13 v.1.1 #issue003 Time zone handling
- Time Zone selection in captive portal is based on human-readable values
- Time Zone setting is now done is FSM

2025-09-13 v.1.0 Animations fixed, OpenWeather added
- OpenWeather is used to retrieve the temperature and humidity
- Animations in "scene" data structure defined

2025-09-06 v.0.1 Date and Time with SlotMachine Animation
- Date and Time animiated
- WiFi connectivity
- Captive Portal

2025-09-07 v.0.2 Finite State Machine
- Code refactored to use a SimpleFSM library

*/

/* ToDo
*/

#endif  // VERSION_H
