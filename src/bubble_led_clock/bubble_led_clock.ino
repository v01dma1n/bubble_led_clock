#define ESP32DEBUGGING
#include <ESP32Logger.h>
#include <WiFi.h>
#include "RTClib.h" // Add this line

#include "DispDriverHT16K33.h"
#include "Clock.h"
#include "AnimSlotMachine.h"
#include "AnimMatrix.h"
#include "AnimScrollingText.h"

#include "access_point.h"
#include "sntp_timer.h"
#include "app_pref.h"
#include "bubble_led_clock.h"

enum ClockState {
  STATE_WIFI_CONNECT,
  STATE_NTP_SYNC,
  STATE_AP_MODE,
  STATE_RUNNING_NORMAL
};
ClockState currentState;
unsigned long stateEntryTime = 0; // To track time spent in a state (for timeouts)
bool apInitialized = false;     // To ensure AP mode is set up only once
bool connectionAttempted = false;

RTC_DS1307 rtc; // Add this line for the DS1307
bool rtcActive = false; // Add this flag

bool wifiActive = false;
bool sntpActive = false;

unsigned long lastAnimationTime = 0;
const unsigned long animationInterval = 5000; // 5 seconds 

DispDriverHT16K33 display(0x70, 8);
Clock myClock(display);


void i2c_scanner_loop() {
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++) {
    // The I2C scanner uses the return value of
    // Wire.endTransmission to see if a device acknowledged
    // the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");
      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  delay(5000); // Wait 5 seconds for next scan
}

String ip2Str(IPAddress ip) {
  String s = "";
  for (int i = 0; i < 4; i++) {
    s += i ? "." + String(ip[i]) : String(ip[i]);
  }
  return s;
}

bool WiFiConnect()
{
    delay(10);

    Serial.println("------------");
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(appPrefs.config.ssid);

    WiFi.setHostname(AP_HOST_NAME);
    WiFi.begin(appPrefs.config.ssid, appPrefs.config.password);

    unsigned waitCnt = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        waitCnt++;
        if (waitCnt > 10) {
          Serial.printf("Connection timed out\n");
          return false;
        }
    }

    IPAddress ip = WiFi.localIP();

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: "); Serial.println(ip);

    return true;
}


void setup() {
  Serial.begin(1152200);
  delay(1000);
  Serial.println("\nStarting...");

  DBGINI(&Serial, ESP32Timestamp::TimestampSinceStart);
  DBGLEV(Debug);
  DBGSTA;
  DBGLOG(Info, "Starting");

  appPrefs.setup();
  appPrefs.getPreferences();
  appPrefs.dumpPreferences();
  Log.setLogLevel(ESP32LogLevel(appPrefs.config.logLevel));

  // Initialize hardware
  myClock.begin();
  display.setBrightness(1);

  // Initialize RTC
  if (!rtc.begin()) {
    DBGLOG(Error, "Couldn't find RTC module!");
  } else {
    DBGLOG(Info, "RTC module found.");
    rtcActive = true;
  }
  
  // Set the initial state and start the process
  currentState = STATE_WIFI_CONNECT;
  DBGLOG(Info, "Entering STATE_WIFI_CONNECT");
}


void formatTime(char *txt, unsigned txt_size, const char *format) {
   time_t now;
  
  if (rtcActive) {
    // If the RTC is working, use it as the definitive time source
    now = rtc.now().unixtime();
  } else {
    // Otherwise, fall back to the ESP32's internal clock
    now = time(0);
  }

  struct tm  timeinfo = *localtime(&now); //
  strftime(txt, txt_size, format, &timeinfo);
  if (txt[0] == '0')
    txt[0] = ' '; // suppress leading zero //
}


void dispTimeFormatted(char *format, unsigned offset=0) {
  char txt[8+1];
  formatTime(txt, sizeof(txt), format);

  // dispClear();
  // dispSetString(txt, offset);
}

void loop() {
  // This must be called every loop to drive animations and the display
  myClock.update();

  switch (currentState) {
  case STATE_WIFI_CONNECT: {
      // This runs only once on the first entry into this state
      if (!connectionAttempted) { 
        DBGLOG(Info, "Starting WiFi connection attempt...");
        // Start the non-blocking connection process [cite: 31]
        WiFi.setHostname(AP_HOST_NAME);
        WiFi.begin(appPrefs.config.ssid, appPrefs.config.password);
        stateEntryTime = millis();
        connectionAttempted = true;
      }

      // If the animation is not running (because it's the first time or it just finished),
      // start a new one. This creates the looping effect.
      if (!myClock.isAnimationRunning()) {
        auto connectingMsg = std::make_unique<ScrollingTextAnimation>("CONNECTING TO WIFI..."); 
        myClock.setAnimation(std::move(connectingMsg));
      }

      // Check for an exit condition (success or timeout) on every loop
      bool timedOut = (millis() - stateEntryTime > 15000);

      if (WiFi.status() == WL_CONNECTED) {
        DBGLOG(Info, "WiFi connected. Entering STATE_NTP_SYNC");
        currentState = STATE_NTP_SYNC; 
        stateEntryTime = millis(); 
        setupSntp();
        connectionAttempted = false; 
      } else if (timedOut) {
        DBGLOG(Error, "WiFi timed out. Entering STATE_AP_MODE");
        currentState = STATE_AP_MODE;
        connectionAttempted = false; 
      }
      break;
    }

case STATE_NTP_SYNC: {
      // Show a syncing message while we wait for NTP
      if (!myClock.isAnimationRunning()) {
        auto syncingMsg = std::make_unique<ScrollingTextAnimation>("WAITING FOR NTP SYNC...");
        myClock.setAnimation(std::move(syncingMsg));
      }

      // Check if time is synced
      if (time(nullptr) > 1000000000) {
        DBGLOG(Info, "NTP sync successful. Updating RTC and entering STATE_RUNNING_NORMAL");
        if (rtcActive) {
          rtc.adjust(DateTime(time(nullptr)));
        }
        sntpActive = true;
        currentState = STATE_RUNNING_NORMAL;
        myClock.setAnimation(nullptr); // Corrected line
        break;
      }

      // Check for a 15-second timeout
      if (millis() - stateEntryTime > 15000) {
        DBGLOG(Error, "NTP sync timed out.");
        if (rtcActive && rtc.isrunning()) {
            DBGLOG(Info, "Using time from RTC module. Entering STATE_RUNNING_NORMAL");
            timeval tv;
            tv.tv_sec = rtc.now().unixtime();
            settimeofday(&tv, nullptr);
            currentState = STATE_RUNNING_NORMAL;
        } else {
            DBGLOG(Error, "No valid time source. Entering STATE_AP_MODE");
            currentState = STATE_AP_MODE;
        }
        myClock.setAnimation(nullptr); // Corrected line
      }
      break;
    }

case STATE_AP_MODE: {
      // This block runs only once to initialize the Access Point and start the timer
      if (!apInitialized) {
        DBGLOG(Info, "Initializing Access Point Mode for 30 seconds.");
        setupAP(AP_HOST_NAME);
        String msg = (String)"CONNECT TO " + AP_HOST_NAME + " WIFI TO CONFIGURE";
        auto apMessage = std::make_unique<ScrollingTextAnimation>(msg.c_str());
        myClock.setAnimation(std::move(apMessage));
        
        apInitialized = true;
        stateEntryTime = millis(); // Start the 30-second timeout timer
      }

      // Check for the 30-second timeout
      if (millis() - stateEntryTime > 30000) {
        DBGLOG(Info, "AP Mode timed out.");
        
        // Only exit AP mode if we have a valid RTC to fall back on
        if (rtcActive && rtc.isrunning()) {
          DBGLOG(Info, "Valid RTC found. Shutting down AP and switching to normal operation.");

          // --- Clean up and shut down AP Mode ---
          WiFi.softAPdisconnect(true);
          WiFi.mode(WIFI_OFF);
          myClock.setAnimation(nullptr); // Clear the AP message

          // Set the system time from the RTC before switching
          timeval tv;
          tv.tv_sec = rtc.now().unixtime();
          settimeofday(&tv, nullptr);
          
          // Transition to the normal clock state
          currentState = STATE_RUNNING_NORMAL;
          apInitialized = false; // Reset the flag for next time
        } else {
          DBGLOG(Error, "AP timed out, but no valid RTC time is available. Staying in AP mode.");
        }
      }
      break;
    }

    case STATE_RUNNING_NORMAL: {
      // This is your original clock logic
      if (myClock.isAnimationRunning()) {
        return; // Let the current animation finish
      }

      unsigned long currentTime = millis();
      if (currentTime - lastAnimationTime >= animationInterval) {
        char timeCStr[9];
        formatTime(timeCStr, sizeof(timeCStr), "%H %M %S");
        auto timeAnimation = std::make_unique<SlotMachineAnimation>(timeCStr);
        myClock.setAnimation(std::move(timeAnimation));
        lastAnimationTime = currentTime;
      } else {
        char timeCStr[9];
        formatTime(timeCStr, sizeof(timeCStr), "%H.%M.%S");
        display.print(timeCStr);
      }
      break;
    }
  }
}
