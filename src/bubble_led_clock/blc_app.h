#ifndef BLC_APP_H
#define BLC_APP_H

#include <memory>
#include "blc_types.h"
#include "blc_access_point_manager.h"
#include "blc_weather_manager.h"
#include "blc_preferences.h"

#include "RTClib.h"
#include <ESP32NTPClock.h>
#include "scene_manager.h"
#include "boot_manager.h"

#define AP_HOST_NAME "bubble-clock"

class BubbleLedClockApp : public IGenericClock {
public:
   static BubbleLedClockApp& getInstance() {
        static BubbleLedClockApp instance;
        return instance;
    }
    void setup();
    void loop();

    AppPreferences& getPrefs() { return _appPrefs; }

    SceneManager& getSceneManager() { return *_sceneManager; }
    
    float getTempData();
    float getHumidityData();
    
    // --- IGenericClock Overrides ---
    void syncRtcFromNtp() override;
    const char* getAppName() const override;
    void activateAccessPoint() override;
    const char* getSsid() const override { return _appPrefs.config.ssid; }
    const char* getPassword() const override { return _appPrefs.config.password; }
    const char* getTimezone() const override { return _appPrefs.config.time_zone; }
    bool isOkToRunScenes() const override { return _fsmManager->isInState("RUNNING_NORMAL"); }

    const char* getTempUnit() const override { return _appPrefs.config.tempUnit; }
    const char* getOwmApiKey() const override { return _appPrefs.config.owm_api_key; }
    const char* getOwmCity() const override { return _appPrefs.config.owm_city; }
    const char* getOwmStateCode() const override { return _appPrefs.config.owm_state_code; }
    const char* getOwmCountryCode() const override { return _appPrefs.config.owm_country_code; }

    void setWeatherData(const OpenWeatherData& data) override { _currentWeatherData = data; }
    void formatTime(char *txt, unsigned txt_size, const char *format, time_t now) override;
    
    IDisplayDriver& getDisplay() override { return _display; }
    DisplayManager& getClock() override { return _displayManager; }
    RTC_DS1307& getRtc() override { return _rtc; }
    bool isRtcActive() const override { return _rtcActive; }

private:
    BubbleLedClockApp();

    // Core hardware components
    DispDriverHT16K33 _display;
    DisplayManager _displayManager;
    RTC_DS1307 _rtc;
    AppPreferences _appPrefs;
    bool _rtcActive;

    // Manager classes
    std::unique_ptr<BlcAccessPointManager> _apManager; 
    std::unique_ptr<ClockFsmManager> _fsmManager;
    std::unique_ptr<SceneManager> _sceneManager;
    std::unique_ptr<WeatherDataManager> _weatherManager; 
    std::unique_ptr<BootManager> _bootManager;
    
    // App-level state data
    OpenWeatherData _currentWeatherData;
};
#endif // BLC_APP_H

