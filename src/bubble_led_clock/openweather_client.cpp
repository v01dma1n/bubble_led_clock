#include "openweather_client.h"

#define ESP32DEBUGGING
#include "debug.h"
#include "blc_app.h"
#include "blc_preferences.h"

#include <WiFi.h>
#include <HTTPClient.h>
// #include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <Arduino.h>

OpenWeatherData getOpenWeatherData() {
    OpenWeatherData data;
    HTTPClient http;
    // WiFiClientSecure client;
    // client.setInsecure(); // Bypass SSL certificate validation for simplicity
    WiFiClient client;

    String city = BubbleLedClockApp::getInstance().getPrefs().config.owm_city;
    city.replace(" ", "%20"); // the spaces in the cities mess up the URL
    String state = BubbleLedClockApp::getInstance().getPrefs().config.owm_state_code;
    String country = BubbleLedClockApp::getInstance().getPrefs().config.owm_country_code;

    // Build the API request URL from the config file
    String url = "https://api.openweathermap.org/data/2.5/weather?q=";
    if (state.length() > 0) { url += "," + state; }
    if (country.length() > 0) { url += "," + country; }
    url += "&appid=";
    url += BubbleLedClockApp::getInstance().getPrefs().config.owm_api_key;
    url += "&units=";
    url += "imperial";

    LOGMSG(APP_LOG_DEBUG, "Fetching weather from: %s", url.c_str());
    if (http.begin(client, url)) {
        http.setTimeout(15000); // 15-second timeout
        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            int tempIndex = payload.indexOf("\"temp\":");
            int humidityIndex = payload.indexOf("\"humidity\":");
            if (tempIndex != -1 && humidityIndex != -1) {
                // Find the end of the temperature value (the comma)
                int commaIndex = payload.indexOf(',', tempIndex);
                String tempStr = payload.substring(tempIndex + 7, commaIndex);
                data.temperatureF = tempStr.toFloat();

                // Find the end of the humidity value (the closing brace)
                int endIndex = payload.indexOf('}', humidityIndex);
                String humidityStr = payload.substring(humidityIndex + 11, endIndex);
                data.humidity = humidityStr.toFloat();
                
                data.isValid = true;
                LOGMSG(APP_LOG_DEBUG, "Weather Parsed: %.1f F, %.0f%% Humidity", data.temperatureF, data.humidity);
            } else {
                LOGMSG(APP_LOG_DEBUG, "Failed to parse weather JSON: %s", payload.c_str());
            }
        } else {
            LOGMSG(APP_LOG_ERROR, "HTTP GET request failed, error: %s %s", http.errorToString(httpCode).c_str(), http.errorToString(httpCode).c_str());
        }
        http.end();
    } else {
        LOGMSG(APP_LOG_ERROR, "Failed to begin HTTP client.");
    }

    return data;
}
