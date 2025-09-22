#include "blc_access_point.h"

#include "blc_app.h"
#define ESP32DEBUGGING
#include "debug.h"
#include "blc_preferences.h"
#include "blc_types.h"
#include "tz_data.h"

#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include <DNSServer.h>
#include <WiFi.h>
#include <stdexcept>
#include <Arduino.h>

DNSServer *dnsServer;
AsyncWebServer *server;

volatile bool g_isClientConnected;

void onWifiEvent(WiFiEvent_t event) {
    if (event == ARDUINO_EVENT_WIFI_AP_STACONNECTED) {
        LOGMSG(APP_LOG_INFO, "Wi-Fi client connected to AP.");
        g_isClientConnected = true;
    } else if (event == ARDUINO_EVENT_WIFI_AP_STADISCONNECTED) {
        LOGMSG(APP_LOG_INFO, "Wi-Fi client disconnected from AP.");
        g_isClientConnected = false;
    }
}

static const PrefSelectOption tempUnitOptions[] = {
    {"Fahrenheit (°F)", OWM_UNIT_IMPERIAL},
    {"Celsius (°C)", OWM_UNIT_METRIC}
};

// Define an array of form fields
FormField formFields[NUM_FORM_FIELDS];

void initializeFormFields() {
    // Get a reference to the preferences object once
    AppPreferences& appPrefs = BubbleLedClockApp::getInstance().getPrefs();

    // --- WIFI_SSID ---
    formFields[WIFI_SSID].id = "WiFiSSIDInput";
    formFields[WIFI_SSID].name = "WiFi SSID";
    formFields[WIFI_SSID].isMasked = false;
    formFields[WIFI_SSID].validation = VALIDATION_NONE;
    formFields[WIFI_SSID].prefType = PREF_STRING;
    formFields[WIFI_SSID].pref.str_pref = appPrefs.config.ssid;

    // --- PASSWORD ---
    formFields[PASSWORD].id = "PasswordInput";
    formFields[PASSWORD].name = "Password";
    formFields[PASSWORD].isMasked = true;
    formFields[PASSWORD].validation = VALIDATION_NONE;
    formFields[PASSWORD].prefType = PREF_STRING;
    formFields[PASSWORD].pref.str_pref = appPrefs.config.password;

    // --- TIME_ZONE ---
    formFields[TIME_ZONE].id = "TimeZoneInput";
    formFields[TIME_ZONE].name = "Time Zone";
    formFields[TIME_ZONE].isMasked = false;
    formFields[TIME_ZONE].validation = VALIDATION_NONE;
    formFields[TIME_ZONE].prefType = PREF_SELECT;
    formFields[TIME_ZONE].pref.str_pref = appPrefs.config.time_zone;
    formFields[TIME_ZONE].select_options = timezones;
    formFields[TIME_ZONE].num_select_options = num_timezones;

    // --- LOG_LEVEL ---
    formFields[LOG_LEVEL].id = "logLevel";
    formFields[LOG_LEVEL].name = "Log Level";
    formFields[LOG_LEVEL].isMasked = false;
    formFields[LOG_LEVEL].validation = VALIDATION_NONE;
    formFields[LOG_LEVEL].prefType = PREF_ENUM;
    formFields[LOG_LEVEL].pref.enum_pref = &appPrefs.config.logLevel;

    // -- SHOW_STARTUP_ANIMATION -- 
    formFields[SHOW_STARTUP_ANIM].id = "ShowStartupAnimInput";
    formFields[SHOW_STARTUP_ANIM].name = "Show Startup Animation";
    formFields[SHOW_STARTUP_ANIM].isMasked = false;
    formFields[SHOW_STARTUP_ANIM].validation = VALIDATION_NONE;
    formFields[SHOW_STARTUP_ANIM].prefType = PREF_BOOL;
    formFields[SHOW_STARTUP_ANIM].pref.bool_pref = &appPrefs.config.showStartupAnimation;

    // --- OWM_CITY ---
    formFields[OWM_CITY].id = "OwmCityInput";
    formFields[OWM_CITY].name = "OWM City";
    formFields[OWM_CITY].isMasked = false;
    formFields[OWM_CITY].validation = VALIDATION_NONE;
    formFields[OWM_CITY].prefType = PREF_STRING;
    formFields[OWM_CITY].pref.str_pref = appPrefs.config.owm_city;

    // --- OWM_API_KEY ---
    formFields[OWM_API_KEY].id = "OwmApiKeyInput";
    formFields[OWM_API_KEY].name = "OWM API Key";
    formFields[OWM_API_KEY].isMasked = true;
    formFields[OWM_API_KEY].validation = VALIDATION_NONE;
    formFields[OWM_API_KEY].prefType = PREF_STRING;
    formFields[OWM_API_KEY].pref.str_pref = appPrefs.config.owm_api_key;

    // --- OWM_STATE_CODE ---
    formFields[OWM_STATE_CODE].id = "OwmStateCodeInput";
    formFields[OWM_STATE_CODE].name = "OWM State Code";
    formFields[OWM_STATE_CODE].isMasked = false;
    formFields[OWM_STATE_CODE].validation = VALIDATION_NONE;
    formFields[OWM_STATE_CODE].prefType = PREF_STRING;
    formFields[OWM_STATE_CODE].pref.str_pref = appPrefs.config.owm_state_code;

    // --- OWM_COUNTRY_CODE ---
    formFields[OWM_COUNTRY_CODE].id = "OwmCountryCodeInput";
    formFields[OWM_COUNTRY_CODE].name = "OWM Country Code";
    formFields[OWM_COUNTRY_CODE].isMasked = false;
    formFields[OWM_COUNTRY_CODE].validation = VALIDATION_NONE;
    formFields[OWM_COUNTRY_CODE].prefType = PREF_STRING;
    formFields[OWM_COUNTRY_CODE].pref.str_pref = appPrefs.config.owm_country_code;

    // --- TEMP_UNITS ---
    formFields[TEMP_UNITS].id = "TempUnitsInput";
    formFields[TEMP_UNITS].name = "Temperature Unit";
    formFields[TEMP_UNITS].isMasked = false;
    formFields[TEMP_UNITS].validation = VALIDATION_NONE;
    formFields[TEMP_UNITS].prefType = PREF_SELECT;
    formFields[TEMP_UNITS].pref.str_pref = appPrefs.config.tempUnit;
    formFields[TEMP_UNITS].select_options = tempUnitOptions;
    formFields[TEMP_UNITS].num_select_options = 2;    
}

#define PASSWORD_MASKED "************"

#define HTML_JAVASCRIPT_PLACEHOLDER "%JAVASCRIPT_PLACEHOLDER%"
#define HTML_FORM_PLACEHOLDER "%FORM_PLACEHOLDER%"

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Bubble LED Display Settings</title>
    <link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Old+Standard+TT&display=swap"> 
<!--    <link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=MedievalSharp&display=swap"> -->
<!--    <link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Cormorant+Garamond&display=swap"> -->
    %JAVASCRIPT_PLACEHOLDER%/home/voidmain/projects/local/bubble_led_clock/src/bubble_led_clock/bubble_led_clock_app.h
    <style>
        body {
            background-image: url('https://plus.unsplash.com/premium_photo-1667761637876-e704c906927d'); 
            background-color: #f0e68c; /* Optional: Set a background color to ensure image covers the entire background */
            background-repeat: no-repeat;
            background-attachment: fixed;
            background-size: cover;
        }
        h1:first-child {
			font-family: "Cormorant Garamond", "MedievalSharp", "Old Standard TT", "Blackletter", "Old English Text MT", "Times New Roman", serif;
			text-align: center;
			font-weight: bold;
        }
        td:first-child, input[type="submit"] {
            text-align: right;
			font-family: "Cormorant Garamond", "MedievalSharp", "Old Standard TT", "Blackletter", "Old English Text MT", "Times New Roman", serif;
			font-weight: bold;
        }
    </style>
</head>
<body>
    <h1>Bubble LED Clock Settings</h1>
    "%FORM_PLACEHOLDER%"
</body>
</html>)rawliteral";

String generateJavascript(const FormField *formFields, int numFields) {

  String script = R"(
<script>
  window.onload = function() {
)";

  for (int i = 0; i < numFields; ++i) {
    const FormField &field = formFields[i];

    if (field.isMasked) {
      continue;
    }

    script += "    const " + String(field.id) +
              " = document.getElementById(\"" + String(field.id) + "\");\n";
    if (field.prefType == PREF_STRING) {
      script += "    " + String(field.id) + ".value = \"" +
                String(field.pref.str_pref) + "\";\n";
    } else if (field.prefType == PREF_BOOL) {
      // script += "    " + String(field.id) + ".value = \"" +
      //           String(*(field.pref.bool_pref) ? "checked" : "") + "\";\n";
      // Correctly set the checked property of the checkbox
      script += "    " + String(field.id) + ".checked = " +
                String(*(field.pref.bool_pref) ? "true" : "false") + ";\n";
    } else if (field.prefType == PREF_INT) {
      script += "    " + String(field.id) + ".value = \"" +
                String(*(field.pref.int_pref)) + "\";\n";
    } else if (field.prefType == PREF_ENUM) {
      script += "    " + String(field.id) + ".value = \"" +
                String(*(field.pref.enum_pref)) + "\";\n";
    }
  }

  script += R"(
  };
</script>
)";

  return script;
}

String generateForm(const FormField *formFields, int numFields) {

  String form = R"(
<form action="/get">
<table>
)";

  for (int i = 0; i < numFields; ++i) {
    const FormField &field = formFields[i];
    form += "<tr>\n";
    form += "<td>" + String(field.name) + ":</td>\n";
    if (field.prefType == PREF_STRING) {
        String inputTag = "<input type=\"";
        inputTag += (field.isMasked ? "password" : "text");
        inputTag += "\" name=\"" + String(field.name) + "\" id=\"" + String(field.id) + "\"";

        // If the field is masked AND a value is already saved, pre-fill with the placeholder.
        if (field.isMasked && strlen(field.pref.str_pref) > 0) {
            inputTag += " value=\"" + String(PASSWORD_MASKED) + "\"";
        }
        inputTag += ">";
        form += "<td>" + inputTag + "</td>\n";
    } else if (field.prefType == PREF_BOOL) {
      form += "<td><input type=\"" + String("checkbox") + "\" name=\"" +
              String(field.name) + "\" id=\"" + String(field.id) + "\" " +
              (*(field.pref.bool_pref) ? +" checked" : "") + "></td>\n";
      // if (*(field.pref.bool_pref))
      //   form += " checked";
    } else if (field.prefType == PREF_INT) {
      form += "<td><input type=\"" + String("number") + "\" name=\"" +
              String(field.name) + "\" id=\"" + String(field.id) + "\"></td>\n";
    } else if (field.prefType == PREF_ENUM) {
      form += "<td><select name=\"" + String(field.name) + "\" id=\"" +
              String(field.id) + "\">\n";
      form += "  <option value=\"" + String(APP_LOG_ERROR) + "\">Error</option>\n";
      form += "  <option value=\"" + String(APP_LOG_INFO) + "\">Info</option>\n";
      form += "  <option value=\"" + String(APP_LOG_DEBUG) + "\">Debug</option>\n";
      form += "</select></td>\n";
    } else if (field.prefType == PREF_SELECT) {
        form += "<td><select name=\"" + String(field.name) + "\" id=\"" + String(field.id) + "\">\n";
        for (int j = 0; j < field.num_select_options; ++j) {
            form += "  <option value=\"" + String(field.select_options[j].value) + "\"";
            if (strcmp(field.pref.str_pref, field.select_options[j].value) == 0) {
                form += " selected";
            }
            form += ">" + String(field.select_options[j].name) + "</option>\n";
        }
        form += "</select></td>\n";
    } else {
      form += "<\td>\n";
    }
    form += "</tr>\n";
  }

  form += R"(
<tr>
  <td colspan="2"><input type="submit"></td>
</tr>
</table>
</form>
)";

  return form;
}

String assembleHtml(const FormField *formFields, int numFields) {
  String html = String(index_html);

  // Replace Javascript Placeholder
  String generatedJavascript = generateJavascript(formFields, numFields);
  html.replace("%JAVASCRIPT_PLACEHOLDER%", generatedJavascript);

  // Generate and replace Form Placeholder
  String formHtml = generateForm(formFields, numFields);
  html.replace("%FORM_PLACEHOLDER%", formHtml);

  return html;
}

void replaceSubstring(String &str, String &oldStr, String &newStr) {
  size_t pos = str.indexOf(oldStr);
  while (pos != -1) {
    str.replace(oldStr, newStr);
    pos = str.indexOf(oldStr, pos + newStr.length());
  }
}

String loadStringFromPROGMEM(const char *str) {

  size_t len = strlen(str);
  char *buffer = (char *)malloc(len + 1);
  strcpy(buffer, str);
  String s(buffer);
  free(buffer);
  return s;
}

bool validateInput(String inp, FieldValidation validation) {

  bool ret = false;

  switch (validation) {
  case VALIDATION_NONE:
    ret = true;
    break;

  case VALIDATION_INTEGER: 
    ret = true;
    // ToDo: validate integer
    break;
  
  case VALIDATION_IP_ADDRESS:
    IPAddress addr;
    ret = addr.fromString(inp.c_str());
    break;

    // case VALIDATION_ENUM:
    //   int intVal = inp.toInt();
    //   ret = (intVal == ERROR || intVal == INFO || intVal == DEBUG);
    //   break;
  }

  return ret;
}

class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request) {
    // request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {

    LOGMSG(APP_LOG_DEBUG,"handleRequest\n");
    String indexHtml = loadStringFromPROGMEM(index_html);
    BubbleLedClockApp::getInstance().getPrefs().getPreferences();

    String substitutedHtml = assembleHtml(formFields, NUM_FORM_FIELDS);

    request->send(200, "text/html", substitutedHtml);
  }
};

void sendHtmlPage(AsyncWebServerRequest *request) {
  String indexHtml = loadStringFromPROGMEM(index_html);
  BubbleLedClockApp::getInstance().getPrefs().getPreferences();
  LOGMSG(APP_LOG_INFO,".");
  BubbleLedClockApp::getInstance().getPrefs().dumpPreferences();
  String substitutedHtml = assembleHtml(formFields, NUM_FORM_FIELDS);
  request->send(200, "text/html", substitutedHtml);
}

void setupServer() {
  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    LOGMSG(APP_LOG_INFO,".");
    sendHtmlPage(request);
    LOGMSG(APP_LOG_DEBUG,"Client Connected");
  });
  
  server->on("/get", HTTP_GET, [](AsyncWebServerRequest *request) {
    LOGMSG(APP_LOG_INFO, "Received new settings via AP.");
    AppPreferences& appPrefs = BubbleLedClockApp::getInstance().getPrefs();
    bool restart = false;

    for (FormField &field : formFields) {
        if (field.prefType == PREF_BOOL) {
            // For checkboxes, if param is missing, it's false.
            bool isChecked = request->hasParam(field.name);
            *(field.pref.bool_pref) = isChecked;
            LOGMSG(APP_LOG_DEBUG, "Saving bool '%s' = %s", field.name, isChecked ? "true" : "false");
            restart = true;
        } else {
            // For all other types, only process if the parameter was sent.
            if (request->hasParam(field.name)) {
                String val = request->getParam(field.name)->value();
                
                if (field.isMasked && (val.isEmpty() || val == PASSWORD_MASKED)) {
                    continue; // Skip empty/placeholder password fields
                }
                
                // Save the new value directly to the preferences object
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
        appPrefs.putPreferences();
        delay(200);
        ESP.restart();
    }

    // Send a confirmation page to the user
    request->send(200, "text/html", "Settings saved. The device will now restart.");
  });
}

/*
void setupServer() {
  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    LOGMSG(APP_LOG_INFO,".");
    sendHtmlPage(request);
    LOGMSG(APP_LOG_DEBUG,"Client Connected");
  });
  
  server->on("/get", HTTP_GET, [](AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    char msg[150]{};
    bool valid = true;

    // --- Validation block (unchanged) ---
    for (FormField &field : formFields) {
      if (request->hasParam(field.name)) {
        String val = request->getParam(field.name)->value();
        if (!validateInput(val, field.validation)) {
          snprintf(msg, sizeof(msg),
                   "Validation failed for field %s and value '%s'.",
                   field.name, val.c_str());
          LOGMSG(APP_LOG_ERROR,"%s", msg);
          valid = false;
          break;
        }
      }
    }

    if (valid) {
      for (FormField &field : formFields) {
        if (field.prefType == PREF_BOOL) {
            field.value = request->hasParam(field.name) ? "on" : "off";
            field.received = true;
            // --- ADDED DIAGNOSTIC LOG ---
            LOGMSG(APP_LOG_DEBUG, "HANDLER: Field '%s' was %s. Set received value to '%s'.", field.name, request->hasParam(field.name) ? "present" : "absent", field.value.c_str());
        } else {
            if (request->hasParam(field.name)) {
              String val = request->getParam(field.name)->value();
              if (field.isMasked) {
                if (val.isEmpty() || val == PASSWORD_MASKED) {
                    continue;
                }
              }
              field.value = val;
              field.received = true;
            }
        }
      }
    }

    if (strlen(msg) == 0) {
      snprintf(msg, sizeof(msg),
               "Settings saved. Device is restarting.<br><a href=\"/\">Return to Home Page</a>\n");
      processAPInput();
    }
    request->send(200, "text/html", msg);
  });
}
*/
/*
void setupServer() {

  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    LOGMSG(APP_LOG_INFO,".");
    sendHtmlPage(request);
    LOGMSG(APP_LOG_DEBUG,"Client Connected");
  });

  server->on("/get", HTTP_GET, [](AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    char msg[150]{};

    // validate input first
    bool valid = true;
    for (FormField &field : formFields) {
      if (request->hasParam(field.name)) {
        String val = request->getParam(field.name)->value();
        if (!validateInput(val, field.validation)) {
          snprintf(msg, sizeof(msg),
                   "Validation failed for field %s and value '%s'.<br><a "
                   "href=\"/\">Return to Home Page</a> ",
                   field.name, val.c_str());
          LOGMSG(APP_LOG_ERROR,"%s", msg);
          valid = false;
          break; // do not look any further
        }
      }
    }

    if (valid) {
        for (FormField &field : formFields) {
            if (field.prefType == PREF_BOOL) {
                // For checkboxes, we always process them. If the parameter exists, it was
                // checked ("on"). If it doesn't exist, it was unchecked ("off").
                field.value = request->hasParam(field.name) ? "on" : "off";
                field.received = true;
            } else {
                // Standard handling for all other input types
                if (request->hasParam(field.name)) {
                  String val = request->getParam(field.name)->value();

                  if (field.isMasked) {
                    if (val.isEmpty() || val == PASSWORD_MASKED) {
                        continue;
                    }
                  }
                  field.value = val;
                  field.received = true;
                }
            }      
        }
    }

    if (strlen(msg) == 0) {
      snprintf(msg, sizeof(msg),
               "The values entered by you have been successfully sent to the "
               "device <br><a href=\"/\">Return to Home Page</a>\n");
      processAPInput();
    }
    request->send(200, "text/html", msg);
  });
}
*/

void setupAP(const char *hostName) {
  
  initializeFormFields();

  LOGMSG(APP_LOG_INFO,"Setting up AP Mode");
  WiFi.onEvent(onWifiEvent);
  WiFi.mode(WIFI_AP);
  LOGMSG(APP_LOG_INFO,"Host: %s", hostName);
  WiFi.softAP(hostName);
  String apIP = WiFi.softAPIP().toString();
  LOGMSG(APP_LOG_INFO,"AP IP address: %s", apIP.c_str());

  server = new AsyncWebServer(80);
  dnsServer = new DNSServer();

  LOGMSG(APP_LOG_DEBUG,"Setting up Async WebServer");
  setupServer();

  server->begin(); // start the Web Server first (memoy allocation)

  LOGMSG(APP_LOG_DEBUG,"Starting DNS Server");
  dnsServer->start(DNS_SERVER_PORT, "*", WiFi.softAPIP());
  
  server->addHandler(new CaptiveRequestHandler())
      .setFilter(ON_AP_FILTER); // only when requested from AP
  // more handlers...
  
  LOGMSG(APP_LOG_DEBUG,"All Done!");
  BubbleLedClockApp::getInstance().getPrefs().getPreferences();
  BubbleLedClockApp::getInstance().getPrefs().dumpPreferences();
}

using namespace std;

void processAPInput() {
  bool restart{false};

  dnsServer->processNextRequest();
  for (FormField &field : formFields) {
    if (field.received) {
      if (field.prefType == PREF_STRING) {
        strncpy(field.pref.str_pref, field.value.c_str(),
                MAX_PREF_STRING_LEN - 1);
        field.pref.str_pref[MAX_PREF_STRING_LEN - 1] = '\0';

      } else if (field.prefType == PREF_BOOL) {
        *(field.pref.bool_pref) = field.value == "on";
      } else if (field.prefType == PREF_INT) {
        try {
          *(field.pref.int_pref) = std::stoi(field.value.c_str());
        } catch (const std::invalid_argument &e) {
          LOGMSG(APP_LOG_ERROR,"Invalid integer value for field %s: %s", field.name,
                 field.value.c_str());
          *(field.pref.int_pref) = 0; // Set to default 0
          continue;
        } catch (const std::out_of_range &e) {
          LOGMSG(APP_LOG_ERROR,"Integer value out of range for field %s: %s",
                 field.name, field.value.c_str());
          *(field.pref.int_pref) = 0; // Set to default 0
          continue;
        } // try

      } else if (field.prefType == PREF_SELECT) {
        strncpy(field.pref.str_pref, field.value.c_str(),
                MAX_PREF_STRING_LEN - 1);
        field.pref.str_pref[MAX_PREF_STRING_LEN - 1] = '\0';      
        
      } else if (field.prefType == PREF_ENUM) {
        try {
          LOGMSG(APP_LOG_DEBUG,"field.name: %s, field.value: %s", field.name,
                 field.value.c_str());
          *(field.pref.enum_pref) =
              static_cast<AppLogLevel>(std::stoi(field.value.c_str()));
        } catch (const std::invalid_argument &e) {
          LOGMSG(APP_LOG_ERROR,"Invalid enum value for field %s: %s", field.name,
                 field.value.c_str());
          *(field.pref.enum_pref) = APP_LOG_ERROR; // Set to default ERROR
          continue;
        } catch (const std::out_of_range &e) {
          LOGMSG(APP_LOG_ERROR,"Enum value out of range for field %s: %s", field.name,
                 field.value.c_str());
          *(field.pref.enum_pref) = APP_LOG_ERROR; // Set to default ERROR
          continue;
        } // try

      }
      field.received = false;
      restart = true;
    }
  }
  if (restart) {
    LOGMSG(APP_LOG_DEBUG,"AP Restarting");
    BubbleLedClockApp::getInstance().getPrefs().putPreferences();
    delay(100);
    ESP.restart();
  }
}

/*
void processAPInput() {
  bool restart{false};

  dnsServer->processNextRequest();

  for (FormField &field : formFields) {
    if (field.received) {
      if (field.prefType == PREF_STRING) {
        strncpy(field.pref.str_pref, field.value.c_str(),
                MAX_PREF_STRING_LEN - 1);
        field.pref.str_pref[MAX_PREF_STRING_LEN - 1] = '\0';

      } else if (field.prefType == PREF_BOOL) {
        *(field.pref.bool_pref) = field.value == "on";

      } else if (field.prefType == PREF_INT) {
        try {
          *(field.pref.int_pref) = std::stoi(field.value.c_str());
        } catch (const std::invalid_argument &e) {
          LOGMSG(APP_LOG_ERROR,"Invalid integer value for field %s: %s", field.name,
                 field.value.c_str());
          *(field.pref.int_pref) = 0; // Set to default 0
          continue;
        } catch (const std::out_of_range &e) {
          LOGMSG(APP_LOG_ERROR,"Integer value out of range for field %s: %s",
                 field.name, field.value.c_str());
          *(field.pref.int_pref) = 0; // Set to default 0
          continue;
        } // try

      } else if (field.prefType == PREF_SELECT) {
        strncpy(field.pref.str_pref, field.value.c_str(),
                MAX_PREF_STRING_LEN - 1);
        field.pref.str_pref[MAX_PREF_STRING_LEN - 1] = '\0';      
        
      } else if (field.prefType == PREF_ENUM) {
        try {
          LOGMSG(APP_LOG_DEBUG,"field.name: %s, field.value: %s", field.name,
                 field.value.c_str());
          *(field.pref.enum_pref) =
              static_cast<AppLogLevel>(std::stoi(field.value.c_str()));
        } catch (const std::invalid_argument &e) {
          LOGMSG(APP_LOG_ERROR,"Invalid enum value for field %s: %s", field.name,
                 field.value.c_str());
          *(field.pref.enum_pref) = APP_LOG_ERROR; // Set to default ERROR
          continue;
        } catch (const std::out_of_range &e) {
          LOGMSG(APP_LOG_ERROR,"Enum value out of range for field %s: %s", field.name,
                 field.value.c_str());
          *(field.pref.enum_pref) = APP_LOG_ERROR; // Set to default ERROR
          continue;
        } // try

      }
      field.received = false;
      restart = true;
    }
  }
  if (restart) {
    LOGMSG(APP_LOG_DEBUG,"AP Restarting");
    BubbleLedClockApp::getInstance().getPrefs().putPreferences();
    delay(100);
    ESP.restart();
  }
}
*/
unsigned long apBootMillis = millis();

void loopAP(unsigned long apRebootTimeMillis) {
  unsigned long curr_millis = millis();
  bool restart{false};

  dnsServer->processNextRequest();

  for (FormField &field : formFields) {
    if (field.received) {
      if (field.prefType == PREF_STRING)
        ; // field.value = field.value;  // this is nonsense :)
      else if (field.prefType == PREF_BOOL)
        *(field.pref.bool_pref) = field.value == "true";
      else if (field.prefType == PREF_INT) {
        try {
          *(field.pref.int_pref) = std::stoi(
              field.value.c_str()); // Use std::stoi for safe conversion
          LOGMSG(APP_LOG_DEBUG,"PREF_INT field %s: %s (%ld)", field.name,
                 field.value.c_str(), *(field.pref.int_pref));
        } catch (const std::invalid_argument &e) {
          LOGMSG(APP_LOG_ERROR,"Invalid integer value for field %s: %s", field.name,
                 field.value.c_str());
          *(field.pref.int_pref) = 0; // Set to default 0
          continue;
        } catch (const std::out_of_range &e) {
          LOGMSG(APP_LOG_ERROR,"Integer value out of range for field %s: %s",
                 field.name, field.value.c_str());
          *(field.pref.int_pref) = 0; // Set to default 0
          continue;
        } // try
      } else if (field.prefType == PREF_ENUM) {
        try {
          LOGMSG(APP_LOG_ERROR,"field.name: %s, field.value: %s", field.name,
                 field.value);
          *(field.pref.enum_pref) =
              static_cast<AppLogLevel>(std::stoi(field.value.c_str()));
          LOGMSG(APP_LOG_ERROR,"PREF_ENUM field %s: %s (%d)", field.name,
                 field.value.c_str(), *(field.pref.enum_pref));
        } catch (const std::invalid_argument &e) {
          LOGMSG(APP_LOG_ERROR,"Invalid enum value for field %s: %s", field.name,
                 field.value.c_str());
          *(field.pref.enum_pref) = APP_LOG_ERROR; // Set to default ERROR
          continue;
        } catch (const std::out_of_range &e) {
          LOGMSG(APP_LOG_ERROR,"Enum value out of range for field %s: %s", field.name,
                 field.value.c_str());
          *(field.pref.enum_pref) = APP_LOG_ERROR; // Set to default ERROR
          continue;
        }
      }
      field.received = false;
      restart = true;
    }
  }

  if (restart) {
    LOGMSG(APP_LOG_DEBUG,"Restarting");
    BubbleLedClockApp::getInstance().getPrefs().putPreferences();
    ESP.restart();
  } else if (curr_millis >= apBootMillis + apRebootTimeMillis) {
    LOGMSG(APP_LOG_DEBUG,"Restarting AP after %ld milliseconds", apBootMillis);
    ESP.restart();
  }
}