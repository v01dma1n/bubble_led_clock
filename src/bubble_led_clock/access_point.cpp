#include <stdexcept>

#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include <DNSServer.h>
#include <WiFi.h>

#define ESP32DEBUGGING
#include <ESP32Logger.h>

#include "access_point.h"
// #include "ae_fsm_config.h"
#include "app_pref.h"

DNSServer *dnsServer;
AsyncWebServer *server;

// Define an array of form fields
FormField formFields[NUM_FORM_FIELDS] = {
    {"WiFiSSIDInput",
     "WiFi SSID",
     false,
     VALIDATION_NONE,
     PREF_STRING,
     {.str_pref = appPrefs.config.ssid},
     "",
     false},
    {"PasswordInput",
     "Password",
     true,
     VALIDATION_NONE,
     PREF_STRING,
     {.str_pref = appPrefs.config.password},
     "",
     false},
    {"TimeZoneInput",
     "Time Zone",
     false,
     VALIDATION_NONE,
     PREF_STRING,
     {.str_pref = appPrefs.config.time_zone},
     "",
     false},
    {"logLevel",
     "Log Level",
     false,
     VALIDATION_NONE,
     PREF_ENUM,
     {.int_pref = &appPrefs.config.logLevel},
     "",
     false},
};

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
    %JAVASCRIPT_PLACEHOLDER%
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
    <h1>Aethereal Echo Gauge Settings</h1>
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
    if (field.prefType == PREF_STRING)
      form += "<td><input type=\"" +
              (field.isMasked ? String("password") : "text") + "\" name=\"" +
              String(field.name) + "\" id=\"" + String(field.id) + "\"></td>\n";
    else if (field.prefType == PREF_BOOL) {
      form += "<td><input type=\"" + String("checkbox") + "\" name=\"" +
              String(field.name) + "\" id=\"" + String(field.id) + "\" " +
              (*(field.pref.bool_pref) ? +" checked" : "") + "></td>\n";
      // if (*(field.pref.bool_pref))
      //   form += " checked";
    } else if (field.prefType == PREF_INT)
      form += "<td><input type=\"" + String("number") + "\" name=\"" +
              String(field.name) + "\" id=\"" + String(field.id) + "\"></td>\n";
    else if (field.prefType == PREF_ENUM) {
      form += "<td><select name=\"" + String(field.name) + "\" id=\"" +
              String(field.id) + "\">\n";
      form += "  <option value=\"" + String(ERROR) + "\">Error</option>\n";
      form += "  <option value=\"" + String(INFO) + "\">Info</option>\n";
      form += "  <option value=\"" + String(DEBUG) + "\">Debug</option>\n";
      form += "</select></td>\n";
    } else
      form += "<\td>\n";
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

    DBGLOG(Debug, "handleRequest\n");
    String indexHtml = loadStringFromPROGMEM(index_html);
    appPrefs.getPreferences();

    String substitutedHtml = assembleHtml(formFields, NUM_FORM_FIELDS);

    request->send(200, "text/html", substitutedHtml);
  }
};

void sendHtmlPage(AsyncWebServerRequest *request) {
  String indexHtml = loadStringFromPROGMEM(index_html);
  appPrefs.getPreferences();
  DBGLOG(Debug, ".");
  appPrefs.dumpPreferences();
  String substitutedHtml = assembleHtml(formFields, NUM_FORM_FIELDS);
  request->send(200, "text/html", substitutedHtml);
}

void setupServer() {

  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    DBGLOG(Info, ".");
    sendHtmlPage(request);
    DBGLOG(Info, "Client Connected");
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
          DBGLOG(Debug, "%s", msg);
          valid = false;
          break; // do not look any further
        }
      }
    }

    if (valid) {
      for (FormField &field : formFields) {
        if (request->hasParam(field.name)) {
          String val = request->getParam(field.name)->value();
          field.value = val;
          field.received = true;
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

void setupAP(const char *hostName) {
  DBGLOG(Info, "Setting up AP Mode");
  WiFi.mode(WIFI_AP);
  DBGLOG(Info, "Host: %s", hostName);
  WiFi.softAP(hostName);
  String apIP = WiFi.softAPIP().toString();
  DBGLOG(Info, "AP IP address: %s", apIP);

  server = new AsyncWebServer(80);
  dnsServer = new DNSServer();

  DBGLOG(Debug, "Setting up Async WebServer");
  setupServer();

  server->begin(); // start the Web Server first (memoy allocation)

  DBGLOG(Debug, "Starting DNS Server");
  dnsServer->start(DNS_SERVER_PORT, "*", WiFi.softAPIP());
  
  server->addHandler(new CaptiveRequestHandler())
      .setFilter(ON_AP_FILTER); // only when requested from AP
  // more handlers...
  
  DBGLOG(Info, "All Done!");
  appPrefs.getPreferences();
  appPrefs.dumpPreferences();
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
          DBGLOG(Error, "Invalid integer value for field %s: %s", field.name,
                 field.value.c_str());
          *(field.pref.int_pref) = 0; // Set to default 0
          continue;
        } catch (const std::out_of_range &e) {
          DBGLOG(Error, "Integer value out of range for field %s: %s",
                 field.name, field.value.c_str());
          *(field.pref.int_pref) = 0; // Set to default 0
          continue;
        } // try
      } else if (field.prefType == PREF_ENUM) {
        try {
          DBGLOG(Debug, "field.name: %s, field.value: %s", field.name,
                 field.value);
          *(field.pref.enum_pref) =
              static_cast<LogLevel>(std::stoi(field.value.c_str()));
        } catch (const std::invalid_argument &e) {
          DBGLOG(Error, "Invalid enum value for field %s: %s", field.name,
                 field.value.c_str());
          *(field.pref.enum_pref) = ERROR; // Set to default ERROR
          continue;
        } catch (const std::out_of_range &e) {
          DBGLOG(Error, "Enum value out of range for field %s: %s", field.name,
                 field.value.c_str());
          *(field.pref.enum_pref) = ERROR; // Set to default ERROR
          continue;
        } // try
      }
      field.received = false;
      restart = true;
    }
  }
  if (restart) {
    DBGLOG(Info, "AP Restarting");
    appPrefs.putPreferences();
    // dnsServer.end();
    // ESP.restart();
    // fsm.trigger(tr_ap_config_ok);
  }
}

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
          DBGLOG(Error, "PREF_INT field %s: %s (%d)", field.name,
                 field.value.c_str(), *(field.pref.int_pref));
        } catch (const std::invalid_argument &e) {
          DBGLOG(Error, "Invalid integer value for field %s: %s", field.name,
                 field.value.c_str());
          *(field.pref.int_pref) = 0; // Set to default 0
          continue;
        } catch (const std::out_of_range &e) {
          DBGLOG(Error, "Integer value out of range for field %s: %s",
                 field.name, field.value.c_str());
          *(field.pref.int_pref) = 0; // Set to default 0
          continue;
        } // try
      } else if (field.prefType == PREF_ENUM) {
        try {
          DBGLOG(Debug, "field.name: %s, field.value: %s", field.name,
                 field.value);
          *(field.pref.enum_pref) =
              static_cast<LogLevel>(std::stoi(field.value.c_str()));
          DBGLOG(Error, "PREF_ENUM field %s: %s (%d)", field.name,
                 field.value.c_str(), *(field.pref.enum_pref));
        } catch (const std::invalid_argument &e) {
          DBGLOG(Error, "Invalid enum value for field %s: %s", field.name,
                 field.value.c_str());
          *(field.pref.enum_pref) = ERROR; // Set to default ERROR
          continue;
        } catch (const std::out_of_range &e) {
          DBGLOG(Error, "Enum value out of range for field %s: %s", field.name,
                 field.value.c_str());
          *(field.pref.enum_pref) = ERROR; // Set to default ERROR
          continue;
        }
      }
      field.received = false;
      restart = true;
    }
  }

  if (restart) {
    DBGLOG(Info, "Restarting");
    appPrefs.putPreferences();
    ESP.restart();
  } else if (curr_millis >= apBootMillis + apRebootTimeMillis) {
    DBGLOG(Info, "Restarting AP after %ld milliseconds", apBootMillis);
    ESP.restart();
  }
}