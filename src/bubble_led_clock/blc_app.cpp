#include "blc_app.h"
#include "debug.h"
#include "version.h"             
#include "anim_scrolling_text.h" 

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
 
// In file: blc_app.cpp

void BubbleLedClockApp::setup() {
    _appPrefs.setup();
    _appPrefs.getPreferences();
    _displayManager.begin();
    
    LOGMSG(APP_LOG_INFO, "BubbleLedClockApp::setup()");

    Preferences ap_prefs;
    ap_prefs.begin("ap_mode_check", false);
    
    bool force_ap = ap_prefs.getBool("force_ap", false);

    if (force_ap) {
        // --- Double Reset Path ---
        // A recent reboot was detected. Clear the flag and enter AP mode.
        LOGMSG(APP_LOG_INFO, "Double reset detected. Forcing AP mode.");
        ap_prefs.putBool("force_ap", false);
        ap_prefs.end();

        setupAP(AP_HOST_NAME);
        String apMsgStr = "AP MODE -- CONNECT TO ";
        apMsgStr += AP_HOST_NAME;
        auto apMessage = std::make_unique<ScrollingTextAnimation>(apMsgStr.c_str());
        _displayManager.setAnimation(std::move(apMessage));

        // This blocking loop runs the Access Point until the device is rebooted.
        while (true) {
            getDisplay().print("AP MODE");  
            _displayManager.update();
            processAPInput();
            delay(10);
        }

    } else {
        // --- Normal Boot Path ---
        // Set the flag to true for the 3-second window.
        ap_prefs.putBool("force_ap", true);
        ap_prefs.end();
        delay(3000); // 3-second window for the user to perform a reset

        // If we get here, it was a normal boot. Clear the flag and continue.
        // This is the crucial change: this code now runs *inside* the else block.
        ap_prefs.putBool("force_ap", false);
        ap_prefs.end();
    }
    
    // --- Normal startup sequence continues from here ---
    std::string message = std::string(APP_NAME) + " v" + VERSION_STRING + 
                          " by " + std::string(APP_AUTHOR) + 
                          " (" + std::string(APP_DATE) + "). " +
                          std::string(APP_MESSAGE);

    auto startupMsg = std::make_unique<ScrollingTextAnimation>(message);
    _displayManager.setAnimation(std::move(startupMsg));
    
    while (_displayManager.isAnimationRunning()) {
        _displayManager.update();
        delay(10);
    }

    if (!_rtc.begin()) {
        LOGMSG(APP_LOG_ERROR, "Couldn't find RTC module!");
        _rtcActive = false;
    } else {
        LOGMSG(APP_LOG_INFO, "RTC module found.");
        _rtcActive = true;
    }
    
    _fsmManager = std::make_unique<BlcFsmManager>(*this);
    _sceneManager = std::make_unique<SceneManager>(*this);
    _fsmManager->setup();
    _sceneManager->setup();
    
    LOGMSG(APP_LOG_INFO, "--- SETUP COMPLETE ---");
}

void BubbleLedClockApp::loop() {

    // Tell the managers to do their work
    _fsmManager->update();
    _sceneManager->update();
    _displayManager.update();
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