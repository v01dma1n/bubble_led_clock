#include "blc_access_point_manager.h"
#include "blc_types.h" 
#include "blc_preferences.h" 

#include "openweather_client.h"

static const PrefSelectOption tempUnitOptions[] = {
    {"Fahrenheit", OWM_UNIT_IMPERIAL},
    {"Celsius", OWM_UNIT_METRIC}
};
static const int numTempUnitOptions = sizeof(tempUnitOptions) / sizeof(PrefSelectOption);

void BlcAccessPointManager::initializeFormFields() {
    
    BaseAccessPointManager::initializeFormFields();

    FormField startupAnimField;
    startupAnimField.id = "showStartupAnim";
    startupAnimField.name = "Show Startup Anim";
    startupAnimField.isMasked = false;
    startupAnimField.prefType = PREF_BOOL;
    startupAnimField.pref.bool_pref = &_prefs.config.showStartupAnimation;
    _formFields.push_back(startupAnimField);

    FormField owmCityField;
    owmCityField.id = "owmCity";
    owmCityField.name = "OWM City";
    owmCityField.isMasked = false;
    owmCityField.prefType = PREF_STRING;
    owmCityField.pref.str_pref = _prefs.config.owm_city;
    _formFields.push_back(owmCityField);
    
    FormField owmStateField;
    owmStateField.id = "owmState";
    owmStateField.name = "OWM State Code";
    owmStateField.isMasked = false;
    owmStateField.prefType = PREF_STRING;
    owmStateField.pref.str_pref = _prefs.config.owm_state_code;
    _formFields.push_back(owmStateField);

    FormField owmCountryField;
    owmCountryField.id = "owmCountry";
    owmCountryField.name = "OWM Country Code";
    owmCountryField.isMasked = false;
    owmCountryField.prefType = PREF_STRING;
    owmCountryField.pref.str_pref = _prefs.config.owm_country_code;
    _formFields.push_back(owmCountryField);

    FormField owmApiKeyField;
    owmApiKeyField.id = "owmApiKey";
    owmApiKeyField.name = "OWM API Key";
    owmApiKeyField.isMasked = true;
    owmApiKeyField.prefType = PREF_STRING;
    owmApiKeyField.pref.str_pref = _prefs.config.owm_api_key;
    _formFields.push_back(owmApiKeyField);

    FormField tempUnitField;
    tempUnitField.id = "tempUnit";
    tempUnitField.name = "Temp Unit (imperial/metric)";
    tempUnitField.isMasked = false;
    tempUnitField.prefType = PREF_SELECT; 
    tempUnitField.pref.str_pref = _prefs.config.tempUnit;
    tempUnitField.select_options = tempUnitOptions; // Point to the options array
    tempUnitField.num_select_options = numTempUnitOptions; // Set the number of options
    _formFields.push_back(tempUnitField);

}