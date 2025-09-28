#ifndef BLC_WEATHER_MANAGER_H
#define BLC_WEATHER_MANAGER_H

class BubbleLedClockApp;

class WeatherDataManager {
public:
    WeatherDataManager(BubbleLedClockApp& app);
    void update();

private:
    BubbleLedClockApp& _app;
    unsigned long _lastWeatherFetchTime;
    const unsigned long _weatherFetchInterval = 900000; // 15 minutes
};

#endif // BLC_WEATHER_MANAGER_H