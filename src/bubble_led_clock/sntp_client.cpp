#include "sntp_client.h"

#include <esp_sntp.h>
#include <Arduino.h>
#include <time.h>

const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";

const long gmtOffset_sec = -3600 * 5;
const int  daylightOffset_sec = 3600;
const char *time_zone =
    "EST5EDT,M3.2.0,M11.1.0"; // TimeZone rule including daylight adjustment
                              // rules (optional)

bool timeAvail = false;

int lastMillis = 0;

int loopSntpGetTime(unsigned intervalMillis) {
  unsigned long currMillis = millis();

  if (currMillis - lastMillis >= intervalMillis) {
    lastMillis += intervalMillis;
    struct tm timeinfo;
    return getLocalTime(&timeinfo);
  }
  return 0;
}

// Callback function (get's called when time adjusts via NTP)
void timeAvailable(struct timeval *t) {
  timeAvail = true;

  struct tm timeinfo;
  getLocalTime(&timeinfo);
}

void setupSntp(const char* tz) {
  // set notification call-back function
  sntp_set_time_sync_notification_cb(timeAvailable);

  /**
   * NTP server address could be aquired via DHCP,
   *
   * NOTE: This call should be made BEFORE esp32 aquires IP address via DHCP,
   * otherwise SNTP option 42 would be rejected by default.
   * NOTE: configTime() function call if made AFTER DHCP-client run
   * will OVERRIDE aquired NTP server address
   */
  esp_sntp_servermode_dhcp(1); // (optional)

  /**
   * This will set configured ntp servers and constant TimeZone/daylightOffset
   * should be OK if your time zone does not need to adjust daylightOffset twice
   * a year, in such a case time adjustment won't be handled automagicaly.
   */
  // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
  configTzTime(time_zone, ntpServer1, ntpServer2);

  /**
   * A more convenient approach to handle TimeZones with daylightOffset
   * would be to specify a environmnet variable with TimeZone definition
   * including daylight adjustmnet rules. A list of rules for your zone could be
   * obtained from
   * https://github.com/esp8266/Arduino/blob/master/cores/esp8266/TZ.h
   */
  configTzTime(time_zone, ntpServer1, ntpServer2);
}
