#ifndef I_GENERIC_CLOCK_H
#define I_GENERIC_CLOCK_H

#include "display_manager.h"
#include "blc_preferences.h" // We need AppPreferences for now
#include "RTClib.h"

// This is an abstract base class (an "interface") that defines
// the essential functions a clock must have for the FSM to control it.
class IGenericClock {
public:
    virtual ~IGenericClock() {}

    // Functions the FSM needs to access
    virtual AppPreferences& getPrefs() = 0;
    virtual DisplayManager& getClock() = 0;
    virtual RTC_DS1307& getRtc() = 0;
    virtual bool isRtcActive() const = 0;
    virtual void syncRtcFromNtp() = 0;
    virtual const char* getAppName() const = 0;
    virtual void activateAccessPoint() = 0;
};

#endif // I_GENERIC_CLOCK_H