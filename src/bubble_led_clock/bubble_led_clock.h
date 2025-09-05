// #include "led_rgb.h"

// #define GPIO_PWM 15

// #define GPIO_SWITCH 21
// #define GPIO_LAMP 23
// #define GPIO_BUZZER 19

// #define GPIO_LED1_R 17
// #define GPIO_LED1_G 5
// #define GPIO_LED1_B 16

// #define GPIO_LED2_R 32 // 25
// #define GPIO_LED2_G 26
// #define GPIO_LED2_B 33

#define GPIO_LED3    4

#define WIFI_CONN_ATTEMPTS 20
#define AP_REBOOT_TIME_MILLIS                                                  \
  10 * 60 * 1000 // how frequently to reboot Access Point if no connection in ms
#define NTP_REFRESH_INTERVAL_MILLIS 30 * 60 * 1000
#define TOUCH_TIMER_MILLIS 100
#define MAX_PING_INTERVAL_SEC 60

#define AP_HOST_NAME "bubble-led"

// Meter Output values
#define AE_OUTPUT_ZERO 0
#define AE_OUTPUT_MAX 1000
#define AE_OUTPUT_MID 30

#define AE_OUTPUT_YES 12
#define AE_OUTPUT_NO 100

#define AE_BEACON_COUNT 4
#define AE_OUTPUT_BEACON1 3.5
#define AE_OUTPUT_BEACON2 8.5
#define AE_OUTPUT_BEACON3 120
#define AE_OUTPUT_BEACON4 350

#define AE_NO_WIFI AE_OUTPUT_ZERO
#define AE_PING_TIMEOUT AE_OUTPUT_MAX
#define AE_AP_RUNNING AE_OUTPUT_MID

// #include "meter_output.h"

// extern MeterOutput meterOutTimer;
extern bool wifiActive;
// extern LedRGB pingRGBTimer1;
// extern LedRGB pingRGBTimer2;

// extern RGB pingIPColors[AE_BEACON_COUNT];
// extern float pingIPMeter[AE_BEACON_COUNT];

#define DEBUG

#ifdef DEBUG
// #define DEBUG_SIMULATED_PING
#define DEBUG_SHORT_AP
#define DEBUG_SHORT_PING
#define DEBUG_FSM_GRAPH
// #define DEBUG_SIMULATED_PING_SERVER
#endif
