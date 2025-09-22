#include "blc_app.h"
#include "debug.h"
#include "version.h"             

#include "anim_scrolling_text.h" 
#include "clock_fsm_manager.h"

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

BubbleLedClockApp::BubbleLedClockApp() :
    _display(0x70, 8),
    _displayManager(_display),
    _rtc(),
    _appPrefs(),
    _apManager(std::make_unique<AccessPointManager>(_appPrefs)) 
{
    _rtcActive = false;
}
 
#define DOUBLE_RESET_WINDOW_S 5 // 5-second window for double reset

void BubbleLedClockApp::setup() {
    _appPrefs.setup();

    if (!_rtc.begin()) {
        LOGMSG(APP_LOG_ERROR, "Couldn't find RTC module!");
        _rtcActive = false;
    } else {
        LOGMSG(APP_LOG_INFO, "RTC module found.");
        _rtcActive = true;
    }
    _displayManager.begin();
    
    LOGMSG(APP_LOG_INFO, "BubbleLedClockApp::setup()");
    _appPrefs.dumpPreferences();
    
    Preferences ap_prefs;
    ap_prefs.begin("ap_mode_check", false);
    
    uint32_t last_boot_time_s = ap_prefs.getUInt("last_boot_s", 0);
    uint32_t current_boot_time_s = _rtcActive ? _rtc.now().unixtime() : 0;
    
    ap_prefs.putUInt("last_boot_s", current_boot_time_s);
    ap_prefs.end();

    if (_rtcActive && last_boot_time_s > 0 && (current_boot_time_s - last_boot_time_s) < DOUBLE_RESET_WINDOW_S) {
        // Double reset was detected. Call the function that handles AP mode.
        // This function will loop forever and will not return.
        activateAccessPoint();
    }
    
    LOGMSG(APP_LOG_INFO, "Normal boot detected.");
    
    _fsmManager = std::make_unique<ClockFsmManager>(*this);
    _sceneManager = std::make_unique<SceneManager>(*this);
    _fsmManager->setup();
    _sceneManager->setup();
    
    LOGMSG(APP_LOG_INFO, "--- APP SETUP COMPLETE ---");
}

void BubbleLedClockApp::loop() {

    // Tell the managers to do their work
    _fsmManager->update();
    _sceneManager->update();
    _displayManager.update();
}


void BubbleLedClockApp::activateAccessPoint() {
    // 1. Setup the Access Point
    _apManager->setup(AP_HOST_NAME);

    // 2. Setup the display state machine for AP mode messages
    enum ApDisplayState { WAITING_FOR_CLIENT, CLIENT_CONNECTED };
    ApDisplayState apState = WAITING_FOR_CLIENT;

    // 3. Create and start the initial "waiting" message
    String waitingMsgStr = "AP MODE -- CONNECT TO ";
    waitingMsgStr += getAppName();
    auto waitingAnimation = std::make_unique<ScrollingTextAnimation>(waitingMsgStr.c_str());
    _displayManager.setAnimation(std::move(waitingAnimation));

    // 4. Enter the main AP loop
    while (true) {
        // Check if the client connection state has changed
        if (apState == WAITING_FOR_CLIENT && _apManager->isClientConnected()) {
            // A client has connected, change the message
            apState = CLIENT_CONNECTED;
            auto connectedAnimation = std::make_unique<ScrollingTextAnimation>("CONNECTED -- PENDING SETUP...");
            _displayManager.setAnimation(std::move(connectedAnimation));
        } else if (apState == CLIENT_CONNECTED && !_apManager->isClientConnected()) {
            // The client has disconnected, go back to the waiting message
            apState = WAITING_FOR_CLIENT;
            auto waitingAnimationRetry = std::make_unique<ScrollingTextAnimation>(waitingMsgStr.c_str());
            _displayManager.setAnimation(std::move(waitingAnimationRetry));
        }

        // Keep all managers running
        _displayManager.update(); // Run the current animation
        _apManager->loop();       // Handle DNS requests
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

void BubbleLedClockApp::syncRtcFromNtp() {
    if (!_rtcActive) {
        return; // Don't do anything if there's no RTC
    }
    
    // Get the current system time (which is now synced to NTP and is in UTC)
    time_t now_utc = time(nullptr);
    
    // Adjust the hardware RTC to this new UTC time
    _rtc.adjust(DateTime(now_utc));
    
    LOGINF("RTC synchronized with NTP time.");
}

const char* BubbleLedClockApp::getAppName() const {
    return AP_HOST_NAME;
}