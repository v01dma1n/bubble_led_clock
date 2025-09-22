#ifndef CLOCK_FSM_MANAGER_H
#define CLOCK_FSM_MANAGER_H

#include "SimpleFSM.h"
#include "i_generic_clock.h"

// Forward declarations for the C-style callback wrappers
void fsm_on_enter_startup_anim_wrapper();
void fsm_on_enter_wifi_connect_wrapper();
void fsm_on_enter_ntp_sync_wrapper();
void fsm_on_exit_ntp_sync_wrapper();
void fsm_on_enter_ap_mode_wrapper();
void fsm_on_exit_ap_mode_wrapper();
void fsm_on_enter_running_normal_wrapper();

bool fsm_guard_anim_done_wrapper();
bool fsm_guard_wifi_connected_wrapper();
bool fsm_guard_wifi_timed_out_wrapper();
bool fsm_guard_ntp_success_wrapper();
bool fsm_guard_ntp_timeout_with_rtc_wrapper();
bool fsm_guard_ntp_timeout_no_rtc_wrapper();
bool fsm_guard_ap_timeout_with_rtc_wrapper();

class ClockFsmManager {
public:
    ClockFsmManager(IGenericClock& clock);
    void setup();
    void update();

    bool isInState(const String& stateName) const; 

    // Public member functions for FSM callbacks and guards
    void on_enter_wifi_connect();
    void on_enter_ntp_sync(); 
    void on_exit_ntp_sync();
    void on_enter_ap_mode();
    void on_exit_ap_mode();
    void on_enter_running_normal();
    void on_enter_startup_anim(); 

    bool guard_wifi_connected();
    bool guard_wifi_timed_out();
    bool guard_ntp_success();
    bool guard_ntp_timeout_with_rtc();
    bool guard_ntp_timeout_no_rtc();
    bool guard_ap_timeout_with_rtc();
    bool guard_anim_done();

private:
    IGenericClock& _clock;
    SimpleFSM _fsm;
};

#endif // CLOCK_FSM_MANAGER_H