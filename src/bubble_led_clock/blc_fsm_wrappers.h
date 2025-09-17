#ifndef BLC_FSM_WRAPPERS_H
#define BLC_FSM_WRAPPERS_H

#include "blc_app.h"
#include "blc_fsm_manager.h"

// --- State Callback Wrappers ---
inline void on_enter_wifi_connect_wrapper() { BubbleLedClockApp::getInstance().getFsmManager().on_enter_wifi_connect(); }
inline void on_enter_ntp_sync_wrapper() { BubbleLedClockApp::getInstance().getFsmManager().on_enter_ntp_sync(); }
inline void on_exit_ntp_sync_wrapper() { BubbleLedClockApp::getInstance().getFsmManager().on_exit_ntp_sync(); }
inline void on_enter_ap_mode_wrapper() { BubbleLedClockApp::getInstance().getFsmManager().on_enter_ap_mode(); }
inline void on_exit_ap_mode_wrapper() { BubbleLedClockApp::getInstance().getFsmManager().on_exit_ap_mode(); }
inline void on_enter_running_normal_wrapper() { BubbleLedClockApp::getInstance().getFsmManager().on_enter_running_normal(); }


// --- Guard Condition Wrappers ---
inline bool guard_wifi_connected_wrapper() { return BubbleLedClockApp::getInstance().getFsmManager().guard_wifi_connected(); }
inline bool guard_wifi_timed_out_wrapper() { return BubbleLedClockApp::getInstance().getFsmManager().guard_wifi_timed_out(); }
inline bool guard_ntp_success_wrapper() { return BubbleLedClockApp::getInstance().getFsmManager().guard_ntp_success(); }
inline bool guard_ntp_timeout_with_rtc_wrapper() { return BubbleLedClockApp::getInstance().getFsmManager().guard_ntp_timeout_with_rtc(); }
inline bool guard_ntp_timeout_no_rtc_wrapper() { return BubbleLedClockApp::getInstance().getFsmManager().guard_ntp_timeout_no_rtc(); }
inline bool guard_ap_timeout_with_rtc_wrapper() {  return BubbleLedClockApp::getInstance().getFsmManager().guard_ap_timeout_with_rtc(); }

#endif // BLC_FSM_WRAPPERS_H