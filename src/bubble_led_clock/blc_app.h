#ifndef BLC_APP_H
#define BLC_APP_H

#include <memory>
#include "blc_types.h"
#include "blc_access_point_manager.h"
#include "blc_weather_manager.h"
#include "blc_preferences.h"

#include "RTClib.h"
#include <ESP32NTPClock.h>
#include <base_ntp_clock_app.h> // Include the new base class from the library
#include <i_weather_clock.h>

#define AP_HOST_NAME "bubble-clock"

// The application class now inherits from the generic "engine"
class BubbleLedClockApp : public BaseNtpClockApp, public IWeatherClock {
public:
    // Singleton access method
    static BubbleLedClockApp& getInstance() {
        static BubbleLedClockApp instance;
        return instance;
    }

    // Main setup and loop, which will override the base versions
    void setup() override;
    void loop() override;

    // Public accessors for application-specific data and managers
    AppPreferences& getPrefs() { return _appPrefs; }
    float getTempData();
    float getHumidityData();
    
    // --- Implementation of the IGenericClock interface ---
    // The application must provide all of these concrete implementations.

    // Configuration getters
    const char* getAppName() const override;
    const char* getSsid() const override { return _appPrefs.config.ssid; }
    const char* getPassword() const override { return _appPrefs.config.password; }
    const char* getTimezone() const override { return _appPrefs.config.time_zone; }
    const char* getTempUnit() const override { return _appPrefs.config.tempUnit; }
    const char* getOwmApiKey() const override { return _appPrefs.config.owm_api_key; }
    const char* getOwmCity() const override { return _appPrefs.config.owm_city; }
    const char* getOwmStateCode() const override { return _appPrefs.config.owm_state_code; }
    const char* getOwmCountryCode() const override { return _appPrefs.config.owm_country_code; }

    // State setters/checkers
    void setWeatherData(const OpenWeatherData& data) override { _currentWeatherData = data; }
    bool isOkToRunScenes() const override;

    // Hardware and utility methods
    void syncRtcFromNtp() override;
    void activateAccessPoint() override;
    void formatTime(char *txt, unsigned int txt_size, const char *format, time_t now) override;
    IDisplayDriver& getDisplay() override { return _display; }
    DisplayManager& getClock() override { return _displayManager; }
    RTC_DS1307& getRtc() override { return _rtc; }
    bool isRtcActive() const override { return _rtcActive; }

private:
    // Private constructor for singleton pattern
    BubbleLedClockApp();

    // Application-specific hardware components
    DispDriverHT16K33 _display;
    DisplayManager _displayManager;
    RTC_DS1307 _rtc;
    bool _rtcActive;

    // Application owns its specific preferences and AP manager
    AppPreferences _appPrefs;
    BlcAccessPointManager _apManager;
    
    // App-level managers and state data
    std::unique_ptr<WeatherDataManager> _weatherManager; 
    OpenWeatherData _currentWeatherData;
};

#endif // BLC_APP_H