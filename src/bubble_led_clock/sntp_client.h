#ifndef SNTP_CLIENT_H
#define SNTP_CLIENT_H

extern bool timeAvail;

void setupSntp(const char* tz);

int loopSntpGetTime(unsigned intervalMillis);

// Member    Type  Meaning Range
// tm_sec    int seconds after the minute  0-60*
// tm_min    int minutes after the hour  0-59
// tm_hour   int hours since midnight  0-23
// tm_mday   int day of the month  1-31
// tm_mon    int months since January  0-11
// tm_year   int years since 1900
// tm_wday   int days since Sunday 0-6
// tm_yday   int days since January 1  0-365
// tm_isdst  int Daylight Saving Time flag

#endif // SNTP_CLIENT_H
