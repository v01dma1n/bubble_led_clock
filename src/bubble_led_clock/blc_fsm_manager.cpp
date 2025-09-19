#include "blc_fsm_manager.h"
#include "debug.h"
#include "blc_app.h" 
#include "version.h"
#include "blc_fsm_wrappers.h"
#include "blc_access_point.h"
#include "anim_scrolling_text.h"
#include "sntp_client.h"

#include <WiFi.h>

BlcFsmManager::BlcFsmManager(BubbleLedClockApp& app) : _app(app) {}

void BlcFsmManager::setup() {
    static State states[] = {
        State("STARTUP_ANIM", on_enter_startup_anim_wrapper),
        State("WIFI_CONNECT", on_enter_wifi_connect_wrapper),
        State("NTP_SYNC", on_enter_ntp_sync_wrapper, nullptr, on_exit_ntp_sync_wrapper),
        State("AP_MODE", on_enter_ap_mode_wrapper, nullptr, on_exit_ap_mode_wrapper),
        State("RUNNING_NORMAL", on_enter_running_normal_wrapper) 
    };
   static TimedTransition transitions[] = {
        // After startup animation is done, transition to Wi-Fi connect
        TimedTransition(&states[0], &states[1],   100, nullptr, "", guard_anim_done_wrapper),

        // Transitions from WIFI_CONNECT (state 1)
        TimedTransition(&states[1], &states[2],   100, nullptr, "", guard_wifi_connected_wrapper),
        TimedTransition(&states[1], &states[3], 15000, nullptr, "", guard_wifi_timed_out_wrapper),

        // Transitions from NTP_SYNC (state 2)
        TimedTransition(&states[2], &states[4],   100, nullptr, "", guard_ntp_success_wrapper),
        TimedTransition(&states[2], &states[4], 15000, nullptr, "", guard_ntp_timeout_with_rtc_wrapper),
        TimedTransition(&states[2], &states[3], 15000, nullptr, "", guard_ntp_timeout_no_rtc_wrapper),

        // Transition from AP_MODE (state 3)
        TimedTransition(&states[3], &states[4], 60000, nullptr, "", guard_ap_timeout_with_rtc_wrapper)
    };
    int num_transitions = sizeof(transitions) / sizeof(TimedTransition);
    _fsm.add(transitions, num_transitions);
    _fsm.setInitialState(&states[0]);
}

void BlcFsmManager::update() {
    _fsm.run(50);
}

bool BlcFsmManager::isInState(const String& stateName) const {
    State* currentState = _fsm.getState();
    if (currentState != nullptr) {
        return currentState->getName() == stateName;
    }
    return false;
}

// --- FSM Callback Implementations ---
void BlcFsmManager::on_enter_wifi_connect() {
    LOGMSG(APP_LOG_INFO, "--> FSM entering WIFI_CONNECT state...");
    WiFi.setHostname(AP_HOST_NAME);
    WiFi.begin(_app.getPrefs().config.ssid, _app.getPrefs().config.password);
    auto connectingMsg = std::make_unique<ScrollingTextAnimation>("CONNECTING TO WIFI...");
    _app.getClock().setAnimation(std::move(connectingMsg));
}

void BlcFsmManager::on_enter_ntp_sync() {
    LOGMSG(APP_LOG_INFO, "Entering STATE_NTP_SYNC");
    setupSntp(_app.getPrefs().config.time_zone);
    auto syncingMsg = std::make_unique<ScrollingTextAnimation>("WAITING FOR NTP SYNC...");
    _app.getClock().setAnimation(std::move(syncingMsg));
}

void BlcFsmManager::on_exit_ntp_sync() {
    _app.getClock().setAnimation(nullptr);
}

void BlcFsmManager::on_enter_ap_mode() {
    LOGMSG(APP_LOG_INFO, "Entering STATE_AP_MODE");
    setupAP(AP_HOST_NAME);
    String msg = (String)"CONNECT TO " + AP_HOST_NAME;
    auto apMessage = std::make_unique<ScrollingTextAnimation>(msg.c_str());
    _app.getClock().setAnimation(std::move(apMessage));
}

void BlcFsmManager::on_exit_ap_mode() {
    LOGMSG(APP_LOG_INFO, "Shutting down AP.");
    WiFi.softAPdisconnect(true);
    // WiFi.mode(WIFI_OFF);
    _app.getClock().setAnimation(nullptr);
}

void BlcFsmManager::on_enter_running_normal() {
    LOGMSG(APP_LOG_INFO, "FSM entering RUNNING_NORMAL state. SceneManager will now take over display.");
}

// --- FSM Guard Implementations ---
void BlcFsmManager::on_enter_startup_anim() {
    if (_app.getPrefs().config.showStartupAnimation) {
        std::string message = std::string(APP_NAME) + " v" + VERSION_STRING + 
                              " by " + std::string(APP_AUTHOR) + 
                              " " + std::string(APP_DATE) + ". " +
                              std::string(APP_MESSAGE);

        auto startupMsg = std::make_unique<ScrollingTextAnimation>(message, 250, true);
        _app.getClock().setAnimation(std::move(startupMsg));
    }
}
bool BlcFsmManager::guard_anim_done() { return !_app.getClock().isAnimationRunning(); }
bool BlcFsmManager::guard_wifi_connected() { return WiFi.status() == WL_CONNECTED; }
bool BlcFsmManager::guard_wifi_timed_out() { return WiFi.status() != WL_CONNECTED; }
bool BlcFsmManager::guard_ntp_success() { return timeAvail; }
bool BlcFsmManager::guard_ntp_timeout_with_rtc() { return !timeAvail && _app.isRtcActive(); }
bool BlcFsmManager::guard_ntp_timeout_no_rtc() { return !timeAvail && !_app.isRtcActive(); }
bool BlcFsmManager::guard_ap_timeout_with_rtc() { return _app.isRtcActive(); }

