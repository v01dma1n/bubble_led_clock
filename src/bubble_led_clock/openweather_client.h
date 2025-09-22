#ifndef OPENWEATHER_CLIENT_H
#define OPENWEATHER_CLIENT_H

#include <Arduino.h>

struct OWMConfig {
    String city;
    String state_code;
    String country_code;
    String api_key;
    String temp_unit;
};

struct OpenWeatherData {
  float temperatureF;
  float humidity;
  bool isValid = false; // Flag to check if the data is good
};

// Function to get the latest weather data from OpenWeatherMap
OpenWeatherData getOpenWeatherData(const OWMConfig& config);

#endif // OPENWEATHER_CLIENT_H
