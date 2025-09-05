#define ESP32DEBUGGING
#include <ESP32Logger.h>

#include "Arduino.h"
#include "bubble_led_clock.h"
#include "app_pref.h"
#include "wifi_connect.h"
// #include "melody.h"
#include <ESPping.h>

bool WiFiConnect(const char *host, const char *ssid, const char *pass,
                 const int attempts) {
  delay(10);

  DBGLOG(Debug, "------------");
  DBGLOG(Debug, "Connecting to %s", ssid);

  WiFi.setHostname(host);
  wl_status_t stat = WiFi.begin(ssid, pass);
  DBGLOG(Debug, "WiFi Status: %d", stat);

  unsigned waitCnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    // delay(500);  // ToDo: play melody instead
    // playMelody(GPIO_BUZZER, MELODY_WIFI_CONNECTED);
    DBGLOG(Debug, ".");
    waitCnt++;
    if (waitCnt > attempts) {
      DBGLOG(Debug, "Connection timed out");
      return false;
    }
  }

  IPAddress ip = WiFi.localIP();

  DBGLOG(Debug, "WiFi connected");
  DBGLOG(Debug, "IP address: %s", ip.toString());

  return true;
}
