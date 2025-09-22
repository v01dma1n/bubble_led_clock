#include "blc_scene_manager.h"
#include "debug.h"
#include "blc_app.h"
#include "anim_slot_machine.h"
#include "anim_matrix.h"


float BubbleLedClockApp_getTimeData() {
  // This is a placeholder; time scenes are handled by the formatTime function.
  return 0;
}

float BubbleLedClockApp_getTempData() {
  // Calls the public getter on the app singleton for temperature.
  return BubbleLedClockApp::getInstance().getTempData();
}

float BubbleLedClockApp_getHumidityData() {
  // Calls the public getter on the app singleton for humidity.
  return BubbleLedClockApp::getInstance().getHumidityData();
}

SceneManager::SceneManager(BubbleLedClockApp& app) : _app(app) {}

void SceneManager::setup() {
    static const DisplayScene scenePlaylist[] = {
      { "Time",        "%H.%M.%S", SLOT_MACHINE, false, 10000, 200, 50, BubbleLedClockApp_getTimeData },
      { "Date",        "%b %d",    MATRIX,       true,   5000, 200, 40, BubbleLedClockApp_getTimeData },
      { "Time",        "%H.%M.%S", SLOT_MACHINE, false, 10000, 200, 50, BubbleLedClockApp_getTimeData },
      { "Temperature", "%3.0f F",  MATRIX,       false,  5000, 250, 40, BubbleLedClockApp_getTempData },
      { "Temperature", "%3.0f C",  MATRIX,       false,  5000, 250, 40, BubbleLedClockApp_getTempData },
      { "Time",        "%H.%M.%S", SLOT_MACHINE, false, 10000, 200, 50, BubbleLedClockApp_getTimeData },
      { "Humidity",    "%3.0f PCT",MATRIX,       false,  5000, 250, 40, BubbleLedClockApp_getHumidityData }
    };
    _scenePlaylist = scenePlaylist;
    _numScenes = sizeof(scenePlaylist) / sizeof(DisplayScene);
    _currentSceneIndex = 0;
    _sceneStartTime = 0;    
    _lastWeatherFetchTime = 0;
}

#define AP_MODE_LONG_PRESS_DURATION 3000 // 3 seconds

void SceneManager::update() {
    // This logic only runs if the FSM is in the RUNNING_NORMAL state.
    if (_app.getFsmManager().isInState("RUNNING_NORMAL") == false) {
        return;
    }

    unsigned long currentTime = millis();
    if (_app.getClock().isAnimationRunning()) {
        return;
    }

    const DisplayScene& currentScene = _scenePlaylist[_currentSceneIndex];
    if (currentTime - _sceneStartTime >= currentScene.duration_ms) {

        int nextIndex = _currentSceneIndex;
        String units = _app.getPrefs().config.tempUnit;
        while (true) {
            nextIndex = (nextIndex + 1) % _numScenes;
            const DisplayScene& nextScene = _scenePlaylist[nextIndex];
            
            if (strcmp(nextScene.scene_name, "Temperature") == 0) {
                bool isImperialScene = (strstr(nextScene.format_string, "F") != nullptr);
                if ((units == OWM_UNIT_IMPERIAL && isImperialScene) || (units == OWM_UNIT_METRIC && !isImperialScene)) {
                    break; // This is the correct scene, break the loop.
                }
            } else {
                break; // Not a temperature scene, so it's a valid next scene.
            }
        }
        _currentSceneIndex = nextIndex;
        _sceneStartTime = millis();

        const DisplayScene& newScene = _scenePlaylist[_currentSceneIndex];

        if (newScene.getDataValue == BubbleLedClockApp_getTempData || newScene.getDataValue == BubbleLedClockApp_getHumidityData) {
            if (!_currentWeatherData.isValid || (millis() - _lastWeatherFetchTime >= _weatherFetchInterval)) {
                _lastWeatherFetchTime = millis();

                OWMConfig owm_config;
                owm_config.city = _app.getPrefs().config.owm_city;
                owm_config.state_code = _app.getPrefs().config.owm_state_code;
                owm_config.country_code = _app.getPrefs().config.owm_country_code;
                owm_config.api_key = _app.getPrefs().config.owm_api_key;
                owm_config.temp_unit = _app.getPrefs().config.tempUnit;
                _currentWeatherData = getOpenWeatherData(owm_config);
                
                _app.setWeatherData(_currentWeatherData);
            }
        }

        char buffer[12];
        time_t now = _app.isRtcActive() ? _app.getRtc().now().unixtime() : time(0);
        if (newScene.getDataValue == BubbleLedClockApp_getTimeData) {
            _app.formatTime(buffer, sizeof(buffer), newScene.format_string, now);
        } else {
            if (_currentWeatherData.isValid) {
                float value = newScene.getDataValue();
                snprintf(buffer, sizeof(buffer), newScene.format_string, value);
            } else {
                strncpy(buffer, "NO DATA", sizeof(buffer));
            }
        }

        LOGDBG("Scene: %s - %s", newScene.scene_name, buffer);

        if (newScene.animation_type == SLOT_MACHINE) {
            auto anim = std::make_unique<SlotMachineAnimation>(buffer, newScene.anim_param_1, 2000, newScene.anim_param_2, newScene.dots_with_previous);
            _app.getClock().setAnimation(std::move(anim));
        } else if (newScene.animation_type == MATRIX) {
            auto anim = std::make_unique<MatrixAnimation>(buffer, newScene.anim_param_1, 2000, newScene.anim_param_2, newScene.dots_with_previous);
            _app.getClock().setAnimation(std::move(anim));
        }
    } else {
        char buffer[12];
        time_t now = _app.isRtcActive() ? _app.getRtc().now().unixtime() : time(0);

        if (currentScene.getDataValue == BubbleLedClockApp_getTimeData) {
            _app.formatTime(buffer, sizeof(buffer), currentScene.format_string, now);
        } else {
            if (_currentWeatherData.isValid) {
                float value = currentScene.getDataValue();
                snprintf(buffer, sizeof(buffer), currentScene.format_string, value);
            } else {
                strncpy(buffer, "NO DATA", sizeof(buffer));
            }
        }
        _app.getDisplay().print(buffer, currentScene.dots_with_previous);
    }
}

