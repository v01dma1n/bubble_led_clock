#ifndef OPENWEATHER_CLIENT_H
#define OPENWEATHER_CLIENT_H

struct OpenWeatherData {
  float temperatureF;
  float humidity;
  bool isValid = false; // Flag to check if the data is good
};

// Function to get the latest weather data from OpenWeatherMap
OpenWeatherData getOpenWeatherData();

#endif // OPENWEATHER_CLIENT_H
