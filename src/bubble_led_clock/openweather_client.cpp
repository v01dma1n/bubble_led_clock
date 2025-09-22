#include "openweather_client.h"

#include "debug.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <Arduino.h>

OpenWeatherData getOpenWeatherData(const OWMConfig& config) {
    OpenWeatherData data;
    static HTTPClient http;
    static WiFiClient client;

    String city = config.city;
    city.replace(" ", "%20"); // the spaces in the cities mess up the URL
    String state = config.state_code;
    String country = config.country_code;

    // Build the API request URL from the config file
    String url = "http://api.openweathermap.org/data/2.5/weather?q=";
    url += city; 
    if (state.length() > 0) { url += "," + state; }
    if (country.length() > 0) { url += "," + country; }
    url += "&appid=";
    url += config.api_key;
    url += "&units=";
    url += config.temp_unit;

    LOGMSG(APP_LOG_DEBUG, "Fetching weather from: %s", url.c_str());
    if (http.begin(client, url)) {
        http.setTimeout(15000); // 15-second timeout
        int httpCode = http.GET();

        String payload = http.getString();

        if (httpCode == HTTP_CODE_OK) {
            int tempIndex = payload.indexOf("\"temp\":");
            int humidityIndex = payload.indexOf("\"humidity\":");
            if (tempIndex != -1 && humidityIndex != -1) {
                int commaIndex = payload.indexOf(',', tempIndex);
                String tempStr = payload.substring(tempIndex + 7, commaIndex);
                data.temperatureF = tempStr.toFloat();

                int endIndex = payload.indexOf('}', humidityIndex);
                String humidityStr = payload.substring(humidityIndex + 11, endIndex);
                data.humidity = humidityStr.toFloat();

                data.isValid = true;
                LOGDBG("Weather Parsed: %.1f, %.0f%% Humidity", data.temperatureF, data.humidity);
            } else {
                LOGMSG(APP_LOG_ERROR, "Failed to parse weather JSON: %s", payload.c_str());
            }
        } else {
            // The new error log also includes the payload, which is more helpful for debugging.
            LOGMSG(APP_LOG_ERROR, "HTTP GET request failed, code: %d, response: %s", httpCode, payload.c_str());
        }
        http.end();
    } else {
        LOGMSG(APP_LOG_ERROR, "Failed to begin HTTP client.");
    }

    return data;
}
