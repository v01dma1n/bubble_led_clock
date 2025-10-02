#include "blc_app.h"
#include "debug.h"
#include "version.h"             

#include <ESP32NTPClock.h>
#include <ESP32NTPClock_HT16K33.h>
#include <Wire.h> 

// --- Data getters for the scene playlist ---
float BubbleLedClockApp_getTimeData() { return 0; }
float BubbleLedClockApp_getTempData() { return BubbleLedClockApp::getInstance().getTempData(); }
float BubbleLedClockApp_getHumidityData() { return BubbleLedClockApp::getInstance().getHumidityData(); }

// --- The application's specific scene playlist ---
static const DisplayScene scenePlaylist[] = {
    { "Time",        "%H.%M.%S", SLOT_MACHINE, false, 10000, 200, 50, BubbleLedClockApp_getTimeData },
    { "Date",        "%b %d %Y", SCROLLING,    true,   7000, 300,  0, BubbleLedClockApp_getTimeData },
    { "Time",        "%H.%M.%S", SLOT_MACHINE, false, 10000, 200, 50, BubbleLedClockApp_getTimeData },
    { "Temperature", "%3.0f F",  MATRIX,       false,  5000, 250, 40, BubbleLedClockApp_getTempData },
    { "Temperature", "%3.0f C",  MATRIX,       false,  5000, 250, 40, BubbleLedClockApp_getTempData },
    { "Time",        "%H.%M.%S", SLOT_MACHINE, false, 10000, 200, 50, BubbleLedClockApp_getTimeData },
    { "Humidity",    "%3.0f PCT",MATRIX,       false,  5000, 250, 40, BubbleLedClockApp_getHumidityData }
};
static const int numScenes = sizeof(scenePlaylist) / sizeof(DisplayScene);

// --- Constructor ---
// Initializes members and assigns pointers to the base class
BubbleLedClockApp::BubbleLedClockApp() :
    _display(0x70, 8),
    _displayManager(_display),
    _appPrefs(),
    _apManager(_appPrefs)
{
    // Assign the pointers in the base class AFTER members are constructed
    _prefs = &_appPrefs;
    BaseNtpClockApp::_apManager = &_apManager;
    _rtcActive = false;
}
 
// --- Main Setup and Loop ---
void BubbleLedClockApp::setup() {
    // 1. Initialize application-specific hardware
    i2c_bus_clear();
    _displayManager.begin();
    _rtcActive = _rtc.begin();
    if (_rtcActive && (!_rtc.isrunning() || _rtc.now() < DateTime(F(__DATE__), F(__TIME__)))) {
        _rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    
    // 2. Call the base class's setup engine
    BaseNtpClockApp::setup();
    
    // 3. Initialize application-specific managers
    _weatherManager = std::make_unique<WeatherDataManager>(*this);
    if (_sceneManager) {
        _sceneManager->setup(scenePlaylist, numScenes);
    }
      
    LOGINF("--- APP SETUP COMPLETE ---");
}

void BubbleLedClockApp::loop() {
    // 1. Call the base class's loop engine
    BaseNtpClockApp::loop();

    // 2. Call the update loops for any application-specific managers
    if (_weatherManager) _weatherManager->update();
}

// --- IGenericClock Interface Implementations ---

void BubbleLedClockApp::activateAccessPoint() {
    _apManager.setup(getAppName());
    String waitingMsgStr = "AP MODE -- CONNECT TO ";
    waitingMsgStr += getAppName();
    _apManager.runBlockingLoop(getClock(), waitingMsgStr.c_str(), "CONNECTED -- SETUP...");
}

float BubbleLedClockApp::getTempData() { 
    return _currentWeatherData.isValid ? _currentWeatherData.temperatureF : UNSET_VALUE;
}

float BubbleLedClockApp::getHumidityData() { 
    return _currentWeatherData.isValid ? _currentWeatherData.humidity : UNSET_VALUE;
}

bool BubbleLedClockApp::isOkToRunScenes() const {
    return _fsmManager && _fsmManager->isInState("RUNNING_NORMAL");
}

void BubbleLedClockApp::formatTime(char *txt, unsigned txt_size, const char *format, time_t now) {
    struct tm timeinfo = *localtime(&now);
    strftime(txt, txt_size, format, &timeinfo);
    if (txt[0] == '0') {
        txt[0] = ' ';
    }
}

void BubbleLedClockApp::syncRtcFromNtp() {
    if (!_rtcActive) {
        return;
    }
    time_t now_utc = time(nullptr);
    _rtc.adjust(DateTime(now_utc));
    LOGINF("RTC synchronized with NTP time.");
}

const char* BubbleLedClockApp::getAppName() const {
    return AP_HOST_NAME;
}