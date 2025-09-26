#ifndef BLC_APP_H
#define BLC_APP_H

#include <memory>
#include "blc_types.h"
#include "blc_scene_manager.h"
#include "access_point_manager.h"

#include "RTClib.h"

#include <ESP32NTPClock.h>

#define AP_HOST_NAME "bubble-clock"

class BubbleLedClockApp : public IGenericClock {
public:
    static BubbleLedClockApp& getInstance() {
        static BubbleLedClockApp instance;
        return instance;
    }
    
    void setup();
    void loop();

    // --- Public Accessors for Managers and Components ---
    AppPreferences& getPrefs() { return _appPrefs; }
    DisplayManager& getClock() { return _displayManager; }
    IDisplayDriver& getDisplay() { return _display; }
    RTC_DS1307& getRtc() { return _rtc; }
    ClockFsmManager& getFsmManager() { return *_fsmManager; }
    SceneManager& getSceneManager() { return *_sceneManager; }
    bool isRtcActive() const { return _rtcActive; }
    
    // Public data accessors used by the scene playlist
    float getTempData();
    float getHumidityData();
    void setWeatherData(const OpenWeatherData& data) { _currentWeatherData = data; }
    void formatTime(char *txt, unsigned txt_size, const char *format, time_t now);
    void syncRtcFromNtp() override;
    const char* getAppName() const override;
    void activateAccessPoint() override;

private:
    BubbleLedClockApp();

    // Core hardware components
    DispDriverHT16K33 _display;
    DisplayManager _displayManager;
    RTC_DS1307 _rtc;
    AppPreferences _appPrefs;
    bool _rtcActive;

    // Manager classes
    std::unique_ptr<AccessPointManager> _apManager; 
    std::unique_ptr<ClockFsmManager> _fsmManager;
    std::unique_ptr<SceneManager> _sceneManager;
    
    // App-level state data
    OpenWeatherData _currentWeatherData;
};

#endif // BLC_APP_H