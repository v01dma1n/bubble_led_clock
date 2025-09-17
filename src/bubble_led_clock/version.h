#define APP_NAME "Bubble LED Clock"
#define APP_AUTHOR "v01dma1n"
#define APP_DATE "2025-09-17"

#define VER_MAJOR 1
#define VER_MINOR 4
#define VER_BUILD 0

#define APP_MESSAGE "The clock connects to NTP Server. Double-press Reset for Access Point."

#include <string>

#define VERSION_STRING                                                         \
  (std::to_string(VER_MAJOR) + "." + std::to_string(VER_MINOR) + "." +         \
   std::to_string(VER_BUILD))

/* Known Problems

  #issue002 Temperature and humidity is displayed as truncated NO DATA when OpenWeather is not configured.
  Perhaps we should not display anything if the token is undefined.
  
  #issue004 Debug enum validation is hardcoded in access_point.c
   form += "  <option value=\"" + String(APP_LOG_ERROR) + "\">Error</option>\n";
   form += "  <option value=\"" + String(APP_LOG_INFO) + "\">Info</option>\n";
   form += "  <option value=\"" + String(APP_LOG_DEBUG) + "\">Debug</option>\n";

  #issue005 The time is incorrect when clock starts withoug WiFi.
  It looks like the RTC time does not consider the TZ 

  #issue006 The scrolling animation leaves the last character 
  At the end of the animation, the last character does not disappear

  #issue008 ...

*/

/* Version history

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
