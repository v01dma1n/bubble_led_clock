#include "blc_app.h"
#include "debug.h"
#include "version.h"             
#include "anim_scrolling_text.h" 

#include <Wire.h> 

// This function manually toggles the I2C clock line to force
// any stuck slave device to release the data line.
void i2c_bus_clear() {
    pinMode(SDA, INPUT_PULLUP);
    pinMode(SCL, INPUT_PULLUP);
    delay(5);

    for (int i = 0; i < 9; i++) {
        digitalWrite(SCL, HIGH);
        delay(5);
        digitalWrite(SCL, LOW);
        delay(5);
    }
}

#define AP_MODE_LONG_PRESS_DURATION 3000 // 3 seconds

// --- Constructor with Member Initializer List ---
BubbleLedClockApp::BubbleLedClockApp() :
    _display(0x70, 8),
    _displayManager(_display),
    _rtc(),
    _appPrefs()
{
    _rtcActive = false;
}
 
#define DOUBLE_RESET_WINDOW_S 5 // 5-second window for double reset

void BubbleLedClockApp::setup() {
    LOGMSG(APP_LOG_INFO, "--- APP SETUP BEGIN ---");
    _appPrefs.setup();

    if (!_rtc.begin()) {
        LOGMSG(APP_LOG_ERROR, "Couldn't find RTC module!");
        _rtcActive = false;
    } else {
        LOGMSG(APP_LOG_INFO, "RTC module found.");
        _rtcActive = true;
    }
    _displayManager.begin();
    
    Preferences ap_prefs;
    ap_prefs.begin("ap_mode_check", false);
    
    uint32_t last_boot_time_s = ap_prefs.getUInt("last_boot_s", 0);
    uint32_t current_boot_time_s = _rtcActive ? _rtc.now().unixtime() : 0;
    
    ap_prefs.putUInt("last_boot_s", current_boot_time_s);
    ap_prefs.end();

    if (_rtcActive && last_boot_time_s > 0 && (current_boot_time_s - last_boot_time_s) < DOUBLE_RESET_WINDOW_S) {
        LOGMSG(APP_LOG_INFO, "Double reset detected. Forcing AP mode.");
        ap_prefs.remove("last_boot_s");
        activateAccessPoint();
    }
    
    LOGMSG(APP_LOG_INFO, "Normal boot detected.");
    _appPrefs.dumpPreferences();
      
    _fsmManager = std::make_unique<BlcFsmManager>(*this);
    _sceneManager = std::make_unique<SceneManager>(*this);
    _fsmManager->setup();
    _sceneManager->setup();
    
    LOGMSG(APP_LOG_INFO, "--- APP SETUP COMPLETE ---");
}

/*
void BubbleLedClockApp::setup() {
    LOGMSG(APP_LOG_INFO, "--- APP SETUP BEGIN ---");

    i2c_bus_clear();

    _appPrefs.setup();
    _appPrefs.getPreferences();

    LOGMSG(APP_LOG_INFO, "BubbleLedClockApp::setup()");

    _appPrefs.dumpPreferences();

     if (!_rtc.begin()) {
        LOGMSG(APP_LOG_ERROR, "Couldn't find RTC module!");
        _rtcActive = false;
    } else {
        LOGMSG(APP_LOG_INFO, "RTC module found.");
        _rtcActive = true;
    }

    _displayManager.begin();

     Preferences ap_prefs;
    ap_prefs.begin("ap_mode_check", false);
    
    // Use the RTC's unix timestamp, which survives reboots
    uint32_t last_boot_time_s = ap_prefs.getUInt("last_boot_s", 0);
    uint32_t current_boot_time_s = _rtcActive ? _rtc.now().unixtime() : 0;
    
    LOGMSG(APP_LOG_INFO, "Double-reset check: Last boot time saved = %u", last_boot_time_s);
    LOGMSG(APP_LOG_INFO, "Double-reset check: Current boot time = %u", current_boot_time_s);
    if (last_boot_time_s > 0) {
        LOGMSG(APP_LOG_INFO, "Double-reset check: Time difference = %d seconds", (current_boot_time_s - last_boot_time_s));
    }

    ap_prefs.putUInt("last_boot_s", current_boot_time_s);
    ap_prefs.end();

    if (_rtcActive && last_boot_time_s > 0 && (current_boot_time_s - last_boot_time_s) < DOUBLE_RESET_WINDOW_S) {
        // --- Double Reset Path ---
        LOGMSG(APP_LOG_INFO, "Double reset detected. Forcing AP mode.");
        ap_prefs.remove("last_boot_s"); // Clear the key
        
        activateAccessPoint(); // This function will loop forever until reboot.
    }
    
    LOGMSG(APP_LOG_INFO, "Normal boot detected.");
    
    // --- Normal startup sequence continues from here ---
    std::string message = std::string(APP_NAME) + " v" + VERSION_STRING + 
                          " by " + std::string(APP_AUTHOR) + 
                          " " + std::string(APP_DATE) + ". " +
                          std::string(APP_MESSAGE);

    auto startupMsg = std::make_unique<ScrollingTextAnimation>(message);
    _displayManager.setAnimation(std::move(startupMsg));
    
    while (_displayManager.isAnimationRunning()) {
        _displayManager.update();
        delay(10);
    }
    
    _fsmManager = std::make_unique<BlcFsmManager>(*this);
    _sceneManager = std::make_unique<SceneManager>(*this);
    _fsmManager->setup();
    _sceneManager->setup();
    
    LOGMSG(APP_LOG_INFO, "--- APP SETUP COMPLETE ---");
}
*/
void BubbleLedClockApp::loop() {

    // Tell the managers to do their work
    _fsmManager->update();
    _sceneManager->update();
    _displayManager.update();
}

void BubbleLedClockApp::activateAccessPoint()
{
    setupAP(AP_HOST_NAME);

    enum ApDisplayState { WAITING_FOR_CLIENT, CLIENT_CONNECTED };
    ApDisplayState apState = WAITING_FOR_CLIENT;
    g_isClientConnected = false; // Ensure flag is reset before starting

    String waitingMsgStr = "AP MODE -- CONNECT TO ";
    waitingMsgStr += AP_HOST_NAME;
    waitingMsgStr += " Wi-Fi";
    auto apMessage = std::make_unique<ScrollingTextAnimation>(waitingMsgStr.c_str());
    _displayManager.setAnimation(std::move(apMessage));

    // This blocking loop runs the Access Point until the device is rebooted.
    while (true) {

      if (apState == WAITING_FOR_CLIENT && g_isClientConnected) {
            // A client has connected, change the message
            apState = CLIENT_CONNECTED;
            auto connectedAnimation = std::make_unique<ScrollingTextAnimation>("CONNECTED -- PENDING SETUP...");
            _displayManager.setAnimation(std::move(connectedAnimation));

        } else if (apState == CLIENT_CONNECTED && !g_isClientConnected) {
            // The client has disconnected, go back to the waiting message
            apState = WAITING_FOR_CLIENT;
            auto waitingAnimationRetry = std::make_unique<ScrollingTextAnimation>(waitingMsgStr.c_str());
            _displayManager.setAnimation(std::move(waitingAnimationRetry));
        }          
        _displayManager.update();
        processAPInput();
        delay(10);
    }

}

float BubbleLedClockApp::getTempData() { return _currentWeatherData.temperatureF; }
float BubbleLedClockApp::getHumidityData() { return _currentWeatherData.humidity; }

void BubbleLedClockApp::formatTime(char *txt, unsigned txt_size, const char *format, time_t now) {
    struct tm timeinfo = *localtime(&now);
    strftime(txt, txt_size, format, &timeinfo);
    if (txt[0] == '0') {
        txt[0] = ' ';
    }
}

