#ifndef BLC_FSM_MANAGER_H
#define BLC_FSM_MANAGER_H

#include "SimpleFSM.h"

class BubbleLedClockApp;

class BlcFsmManager {
public:
    BlcFsmManager(BubbleLedClockApp& app);

    void setup();
    void update(); 

    bool isInState(const String& stateName) const; 

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
    BubbleLedClockApp& _app; 
    SimpleFSM _fsm;
};

#endif // BLC_FSM_MANAGER_H