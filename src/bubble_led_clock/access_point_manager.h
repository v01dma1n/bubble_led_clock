#ifndef ACCESS_POINT_MANAGER_H
#define ACCESS_POINT_MANAGER_H

#include "blc_types.h"
#include "ESPAsyncWebServer.h"
#include "DNSServer.h"

#define DNS_SERVER_PORT 53

class AppPreferences;

class AccessPointManager {
public:
    AccessPointManager(AppPreferences& prefs);

    void setup(const char* hostName);
    void loop(); // Handles DNS requests in a blocking loop
    bool isClientConnected() { return _isClientConnected; };
    void setIsClientConnected(bool isClientConnected) { _isClientConnected = isClientConnected; };

private:
    AppPreferences& _prefs;
    DNSServer _dnsServer;
    AsyncWebServer _server;
    FormField _formFields[NUM_FORM_FIELDS];
    bool _isClientConnected = false; 

    void initializeFormFields();
    String generateJavascript();
    String generateForm();
    String assembleHtml();
    void setupServer();
};

#endif // ACCESS_POINT_MANAGER_H
