#include "debug.h"
#include "blc_app.h"
#include "blc_preferences.h"
#include "wifi_connector.h"

#include <ESPping.h>
#include "Arduino.h"


bool WiFiConnect(const char *host, const char *ssid, const char *pass,
                 const int attempts) {
  delay(10);

  LOGMSG(APP_LOG_DEBUG, "------------");
  LOGMSG(APP_LOG_DEBUG, "Connecting to %s", ssid);

  WiFi.setHostname(host);
  wl_status_t stat = WiFi.begin(ssid, pass);
  LOGMSG(APP_LOG_DEBUG, "WiFi Status: %d", stat);

  unsigned waitCnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    // delay(500);  // ToDo: play melody instead
    // playMelody(GPIO_BUZZER, MELODY_WIFI_CONNECTED);
    // LOGMSG(APP_LOG_DEBUG, ".");
    waitCnt++;
    if (waitCnt > attempts) {
      LOGMSG(APP_LOG_DEBUG, "Connection timed out");
      return false;
    }
  }

  IPAddress ip = WiFi.localIP();

  LOGMSG(APP_LOG_DEBUG, "WiFi connected");
  LOGMSG(APP_LOG_DEBUG, "IP address: %s", ip.toString().c_str());

  return true;
}
