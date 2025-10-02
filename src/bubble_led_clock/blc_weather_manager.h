#ifndef BLC_WEATHER_MANAGER_H
#define BLC_WEATHER_MANAGER_H

class IWeatherClock; // Forward-declare the interface

class WeatherDataManager {
public:
    WeatherDataManager(IWeatherClock& app); // Use the interface
    void update();

private:
    IWeatherClock& _app; // Use the interface
    unsigned long _lastWeatherFetchTime;
    const unsigned long _weatherFetchInterval = 900000; // 15 minutes
};

#endif // BLC_WEATHER_MANAGER_H