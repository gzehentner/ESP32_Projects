#ifndef MyWiFiLib_h_
#define MyWiFiLib_h_


#ifndef STASSID                // either use an external .h file containing STASSID and STAPSK or ...
#define STASSID "Zehentner"    // ... modify these line to your SSID
#define STAPSK "ElisabethScho" // ... and set your WIFI password
#endif

extern const char *ssid;
extern const char *password;

void restartWiFi();
void connectWifi();
#endif
