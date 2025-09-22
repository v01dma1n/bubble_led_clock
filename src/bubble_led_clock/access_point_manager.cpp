#include "access_point_manager.h"
#include "blc_preferences.h"
#include "debug.h"
#include "tz_data.h"
#include <WiFi.h>

static AccessPointManager* _apInstance = nullptr;

// a callback for the WiFi library 
void onWifiEvent(WiFiEvent_t event) {
    if (!_apInstance) return;
    if (event == ARDUINO_EVENT_WIFI_AP_STACONNECTED) {
        LOGMSG(APP_LOG_INFO, "Wi-Fi client connected to AP.");
        _apInstance->setIsClientConnected(true);
    } else if (event == ARDUINO_EVENT_WIFI_AP_STADISCONNECTED) {
        LOGMSG(APP_LOG_INFO, "Wi-Fi client disconnected from AP.");
        _apInstance->setIsClientConnected(false);
    }
}

// These options are only used by the AccessPointManager
static const PrefSelectOption tempUnitOptions[] = {
    {"Fahrenheit (°F)", OWM_UNIT_IMPERIAL},
    {"Celsius (°C)", OWM_UNIT_METRIC}
};

#define PASSWORD_MASKED "************"

AccessPointManager::AccessPointManager(AppPreferences& prefs) 
    : _prefs(prefs), _server(80) {
    _apInstance = this; 
}

void AccessPointManager::setup(const char* hostName) {
    initializeFormFields();
    setupServer();

    LOGMSG(APP_LOG_INFO,"Setting up AP Mode");
    WiFi.onEvent(onWifiEvent);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(hostName);
    
    LOGMSG(APP_LOG_INFO,"AP IP address: %s", WiFi.softAPIP().toString().c_str());
    _dnsServer.start(DNS_SERVER_PORT, "*", WiFi.softAPIP());
    _server.begin();
    LOGMSG(APP_LOG_DEBUG,"AP Setup Complete.");
}

void AccessPointManager::loop() {
    // This method should be called repeatedly in a blocking loop
    // to handle captive portal DNS requests.
    _dnsServer.processNextRequest();
}

void AccessPointManager::setupServer() {
    _server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "text/html", assembleHtml());
    });

    _server.on("/get", HTTP_GET, [this](AsyncWebServerRequest *request) {
        LOGMSG(APP_LOG_INFO, "Received new settings via AP.");
        bool restart = false;

        for (FormField &field : _formFields) {
            if (field.prefType == PREF_BOOL) {
                bool isChecked = request->hasParam(field.name);
                *(field.pref.bool_pref) = isChecked;
                LOGMSG(APP_LOG_DEBUG, "Saving bool '%s' = %s", field.name, isChecked ? "true" : "false");
                restart = true;
            } else {
                if (request->hasParam(field.name)) {
                    String val = request->getParam(field.name)->value();
                    if (field.isMasked && (val.isEmpty() || val == PASSWORD_MASKED)) {
                        continue;
                    }
                    if (field.prefType == PREF_STRING || field.prefType == PREF_SELECT) {
                        strncpy(field.pref.str_pref, val.c_str(), MAX_PREF_STRING_LEN - 1);
                        field.pref.str_pref[MAX_PREF_STRING_LEN - 1] = '\0';
                    } else if (field.prefType == PREF_ENUM) {
                        *(field.pref.enum_pref) = static_cast<AppLogLevel>(val.toInt());
                    }
                    LOGMSG(APP_LOG_DEBUG, "Saving field '%s' = %s", field.name, val.c_str());
                    restart = true;
                }
            }
        }

        if (restart) {
            LOGMSG(APP_LOG_INFO, "Settings saved. Restarting device.");
            _prefs.putPreferences();
            delay(200);
            ESP.restart();
        }
        request->send(200, "text/html", "Settings saved. The device will now restart.");
    });
}

void AccessPointManager::initializeFormFields() {
    _formFields[WIFI_SSID].id = "WiFiSSIDInput";
    _formFields[WIFI_SSID].name = "WiFi SSID";
    _formFields[WIFI_SSID].isMasked = false;
    _formFields[WIFI_SSID].validation = VALIDATION_NONE;
    _formFields[WIFI_SSID].prefType = PREF_STRING;
    _formFields[WIFI_SSID].pref.str_pref = _prefs.config.ssid;

    _formFields[PASSWORD].id = "PasswordInput";
    _formFields[PASSWORD].name = "Password";
    _formFields[PASSWORD].isMasked = true;
    _formFields[PASSWORD].validation = VALIDATION_NONE;
    _formFields[PASSWORD].prefType = PREF_STRING;
    _formFields[PASSWORD].pref.str_pref = _prefs.config.password;

    _formFields[TIME_ZONE].id = "TimeZoneInput";
    _formFields[TIME_ZONE].name = "Time Zone";
    _formFields[TIME_ZONE].isMasked = false;
    _formFields[TIME_ZONE].validation = VALIDATION_NONE;
    _formFields[TIME_ZONE].prefType = PREF_SELECT;
    _formFields[TIME_ZONE].pref.str_pref = _prefs.config.time_zone;
    _formFields[TIME_ZONE].select_options = timezones;
    _formFields[TIME_ZONE].num_select_options = num_timezones;

    _formFields[LOG_LEVEL].id = "logLevel";
    _formFields[LOG_LEVEL].name = "Log Level";
    _formFields[LOG_LEVEL].isMasked = false;
    _formFields[LOG_LEVEL].validation = VALIDATION_NONE;
    _formFields[LOG_LEVEL].prefType = PREF_ENUM;
    _formFields[LOG_LEVEL].pref.enum_pref = &_prefs.config.logLevel;

    _formFields[SHOW_STARTUP_ANIM].id = "ShowStartupAnimInput";
    _formFields[SHOW_STARTUP_ANIM].name = "Show Startup Animation";
    _formFields[SHOW_STARTUP_ANIM].isMasked = false;
    _formFields[SHOW_STARTUP_ANIM].validation = VALIDATION_NONE;
    _formFields[SHOW_STARTUP_ANIM].prefType = PREF_BOOL;
    _formFields[SHOW_STARTUP_ANIM].pref.bool_pref = &_prefs.config.showStartupAnimation;

    _formFields[OWM_CITY].id = "OwmCityInput";
    _formFields[OWM_CITY].name = "OWM City";
    _formFields[OWM_CITY].isMasked = false;
    _formFields[OWM_CITY].validation = VALIDATION_NONE;
    _formFields[OWM_CITY].prefType = PREF_STRING;
    _formFields[OWM_CITY].pref.str_pref = _prefs.config.owm_city;

    _formFields[OWM_API_KEY].id = "OwmApiKeyInput";
    _formFields[OWM_API_KEY].name = "OWM API Key";
    _formFields[OWM_API_KEY].isMasked = true;
    _formFields[OWM_API_KEY].validation = VALIDATION_NONE;
    _formFields[OWM_API_KEY].prefType = PREF_STRING;
    _formFields[OWM_API_KEY].pref.str_pref = _prefs.config.owm_api_key;

    _formFields[OWM_STATE_CODE].id = "OwmStateCodeInput";
    _formFields[OWM_STATE_CODE].name = "OWM State Code";
    _formFields[OWM_STATE_CODE].isMasked = false;
    _formFields[OWM_STATE_CODE].validation = VALIDATION_NONE;
    _formFields[OWM_STATE_CODE].prefType = PREF_STRING;
    _formFields[OWM_STATE_CODE].pref.str_pref = _prefs.config.owm_state_code;

    _formFields[OWM_COUNTRY_CODE].id = "OwmCountryCodeInput";
    _formFields[OWM_COUNTRY_CODE].name = "OWM Country Code";
    _formFields[OWM_COUNTRY_CODE].isMasked = false;
    _formFields[OWM_COUNTRY_CODE].validation = VALIDATION_NONE;
    _formFields[OWM_COUNTRY_CODE].prefType = PREF_STRING;
    _formFields[OWM_COUNTRY_CODE].pref.str_pref = _prefs.config.owm_country_code;

    _formFields[TEMP_UNITS].id = "TempUnitsInput";
    _formFields[TEMP_UNITS].name = "Temperature Unit";
    _formFields[TEMP_UNITS].isMasked = false;
    _formFields[TEMP_UNITS].validation = VALIDATION_NONE;
    _formFields[TEMP_UNITS].prefType = PREF_SELECT;
    _formFields[TEMP_UNITS].pref.str_pref = _prefs.config.tempUnit;
    _formFields[TEMP_UNITS].select_options = tempUnitOptions;
    _formFields[TEMP_UNITS].num_select_options = 2;
}

String AccessPointManager::generateForm() {
  String form = "<form action=\"/get\"><table>";
  for (int i = 0; i < NUM_FORM_FIELDS; ++i) {
    const FormField &field = _formFields[i];
    form += "<tr>";
    form += "<td>" + String(field.name) + ":</td>";
    if (field.prefType == PREF_STRING) {
        String inputTag = "<input type=\"";
        inputTag += (field.isMasked ? "password" : "text");
        inputTag += "\" name=\"" + String(field.name) + "\" id=\"" + String(field.id) + "\"";
        if (field.isMasked && strlen(field.pref.str_pref) > 0) {
            inputTag += " value=\"" + String(PASSWORD_MASKED) + "\"";
        }
        inputTag += ">";
        form += "<td>" + inputTag + "</td>";
    } else if (field.prefType == PREF_BOOL) {
      form += "<td><input type=\"checkbox\" name=\"" + String(field.name) + 
              "\" id=\"" + String(field.id) + "\" " +
              (*(field.pref.bool_pref) ? "checked" : "") + "></td>";
    } else if (field.prefType == PREF_INT) {
      form += "<td><input type=\"number\" name=\"" + String(field.name) + 
              "\" id=\"" + String(field.id) + "\"></td>";
    } else if (field.prefType == PREF_ENUM) {
      form += "<td><select name=\"" + String(field.name) + "\" id=\"" + String(field.id) + "\">";
      form += "<option value=\"" + String(APP_LOG_ERROR) + "\">Error</option>";
      form += "<option value=\"" + String(APP_LOG_INFO) + "\">Info</option>";
      form += "<option value=\"" + String(APP_LOG_DEBUG) + "\">Debug</option>";
      form += "</select></td>";
    } else if (field.prefType == PREF_SELECT) {
        form += "<td><select name=\"" + String(field.name) + "\" id=\"" + String(field.id) + "\">";
        for (int j = 0; j < field.num_select_options; ++j) {
            form += "<option value=\"" + String(field.select_options[j].value) + "\"";
            if (strcmp(field.pref.str_pref, field.select_options[j].value) == 0) {
                form += " selected";
            }
            form += ">" + String(field.select_options[j].name) + "</option>";
        }
        form += "</select></td>";
    }
    form += "</tr>";
  }
  form += "<tr><td colspan=\"2\"><input type=\"submit\"></td></tr>";
  form += "</table></form>";
  return form;
}

String AccessPointManager::generateJavascript() {
    String script = "<script>window.onload = function() {";
    for (int i = 0; i < NUM_FORM_FIELDS; ++i) {
        const FormField &field = _formFields[i];
        if (field.isMasked) continue;
        script += "const e" + String(i) + " = document.getElementById(\"" + String(field.id) + "\");";
        if (field.prefType == PREF_STRING) {
            script += "e" + String(i) + ".value = \"" + String(field.pref.str_pref) + "\";";
        } else if (field.prefType == PREF_BOOL) {
            script += "e" + String(i) + ".checked = " + String(*(field.pref.bool_pref) ? "true" : "false") + ";";
        } else if (field.prefType == PREF_INT) {
            script += "e" + String(i) + ".value = \"" + String(*(field.pref.int_pref)) + "\";";
        } else if (field.prefType == PREF_ENUM) {
            script += "e" + String(i) + ".value = \"" + String(*(field.pref.enum_pref)) + "\";";
        }
    }
    script += "};</script>";
    return script;
}

String AccessPointManager::assembleHtml() {
    // This is a simplified version that doesn't use PROGMEM for clarity in this refactor.
    String html = "<!DOCTYPE html><html><head><title>Clock Settings</title>";
    html += generateJavascript();
    html += "</head><body><h1>Clock Settings</h1>";
    html += generateForm();
    html += "</body></html>";
    return html;
}