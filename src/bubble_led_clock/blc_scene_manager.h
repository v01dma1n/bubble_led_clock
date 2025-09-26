#ifndef BLC_SCENE_MANAGER_H
#define BLC_SCENE_MANAGER_H

#include "blc_types.h"

#include <ESP32NTPClock.h>

class BubbleLedClockApp; // Forward-declaration

class SceneManager {
public:
    SceneManager(BubbleLedClockApp& app);
    
    void setup();
    void update();

private:
    BubbleLedClockApp& _app;

    // State for the scene playlist
    const DisplayScene* _scenePlaylist;
    int _numScenes;
    int _currentSceneIndex;
    unsigned long _sceneStartTime;

    // State for weather data
    OpenWeatherData _currentWeatherData;
    unsigned long _lastWeatherFetchTime;
    const unsigned long _weatherFetchInterval = 900000; // 15 minutes

};

#endif // BLC_SCENE_MANAGER_H