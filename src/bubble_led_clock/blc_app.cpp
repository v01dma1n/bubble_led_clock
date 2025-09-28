#include "blc_app.h"
#include "debug.h"
#include "version.h"             

#include <ESP32NTPClock.h>
#include <Wire.h> 

float BubbleLedClockApp_getTimeData() { return 0; } // Placeholder
float BubbleLedClockApp_getTempData() { return BubbleLedClockApp::getInstance().getTempData(); }
float BubbleLedClockApp_getHumidityData() { return BubbleLedClockApp::getInstance().getHumidityData(); }

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
    _apManager(std::make_unique<BlcAccessPointManager>(_appPrefs)) 
{
    _rtcActive = false;
}
 
#define DOUBLE_RESET_WINDOW_S 5 // 5-second window for double reset

void BubbleLedClockApp::setup() {

    // Force-reset the I2C bus to clear any stuck devices, then initialize it.
    i2c_bus_clear();
    Wire.begin();
    
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
    
    _bootManager = std::make_unique<BootManager>(*this);
    if (_bootManager->checkForForceAPMode()) {
        LOGMSG(APP_LOG_INFO, "Double reset detected. Activating AP mode.");
        activateAccessPoint();
        return; // Stop further setup and wait in AP mode
    }
    
    LOGMSG(APP_LOG_INFO, "Normal boot detected.");
    
    _fsmManager = std::make_unique<ClockFsmManager>(*this);
    _sceneManager = std::make_unique<SceneManager>(*this);
    _weatherManager = std::make_unique<WeatherDataManager>(*this);
    _fsmManager->setup();
    _sceneManager->setup(scenePlaylist, numScenes); 
      
    LOGMSG(APP_LOG_INFO, "--- APP SETUP COMPLETE ---");
}

void BubbleLedClockApp::loop() {
    _weatherManager->update();
    _fsmManager->update();
    _sceneManager->update();
    _displayManager.update();
}

void BubbleLedClockApp::activateAccessPoint() {
    _apManager->setup(getAppName());

    String waitingMsgStr = "AP MODE -- CONNECT TO ";
    waitingMsgStr += getAppName();
    
    // This single call handles the entire blocking loop and display logic.
    _apManager->runBlockingLoop(getClock(), waitingMsgStr.c_str(), "CONNECTED -- SETUP...");
}

float BubbleLedClockApp::getTempData() { 
    return _currentWeatherData.isValid ? _currentWeatherData.temperatureF : UNSET_VALUE;
}
float BubbleLedClockApp::getHumidityData() { 
    return _currentWeatherData.isValid ? _currentWeatherData.humidity : UNSET_VALUE;
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

