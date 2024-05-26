#include <Arduino.h>

/*
  NTP TZ DST - bare minimum
  NetWork Time Protocol - Time Zone - Daylight Saving Time

  Our target for this MINI sketch is:
  - get the SNTP request running
  - set the timezone
  - (implicit) respect daylight saving time
  - how to "read" time to be printed to Serial.Monitor
  
  This example is a stripped down version of the NTP-TZ-DST (v2)
  And works for ESP8266 core 2.7.4 and 3.0.2

  by noiasca
  2020-09-22
*/

#ifndef STASSID
#define STASSID "Zehentner"                            // set your SSID
#define STAPSK  "ElisabethScho"                        // set your wifi password
#endif

/* Configuration of NTP */
#define MY_NTP_SERVER "at.pool.ntp.org"           
#define MY_TZ "CET-1CEST,M3.5.0/02,M10.5.0/03"   

/* Necessary Includes */
#include <WiFi.h>            // we need wifi to get internet access
#include <time.h>                   // for time() ctime()

/* Globals */
time_t now;                         // this are the seconds since Epoch (1970) - UTC
tm tm1;                              // the structure tm holds time information in a more convenient way

void showTime() {
  time(&now);                       // read the current time
  localtime_r(&now, &tm1);           // update the structure tm with the current time
  Serial.print("year:");
  Serial.print(tm1.tm_year + 1900);  // years since 1900
  Serial.print("\tmonth:");
  Serial.print(tm1.tm_mon + 1);      // January = 0 (!)
  Serial.print("\tday:");
  Serial.print(tm1.tm_mday);         // day of month
  Serial.print("\thour:");
  Serial.print(tm1.tm_hour);         // hours since midnight  0-23
  Serial.print("\tmin:");
  Serial.print(tm1.tm_min);          // minutes after the hour  0-59
  Serial.print("\tsec:");
  Serial.print(tm1.tm_sec);          // seconds after the minute  0-61*
  Serial.print("\twday");
  Serial.print(tm1.tm_wday);         // days since Sunday 0-6
  if (tm1.tm_isdst == 1)             // Daylight Saving Time flag
    Serial.print("\tDST");
  else
    Serial.print("\tstandard");
  Serial.println();
}

void showTimeAlternative() {
  time_t now;                       // this are the seconds since Epoch (1970) - UTC
  tm tm;                            // the structure tm holds time information in a more convenient way
  time(&now);                       // read the current time
  localtime_r(&now, &tm);           // update the structure tm with the current time
  char buffer[42] {0};              // a buffer large enough to hold your output
  strftime (buffer, sizeof(buffer), "%H:%M", &tm);  // for different formats see https://cplusplus.com/reference/ctime/strftime/
  Serial.println(buffer);
}


void setup() {
  Serial.begin(115200);
  Serial.println("\nNTP TZ DST - bare minimum");

  #ifdef ARDUINO_ARCH_ESP32
    // ESP32 seems to be a little more complex:
    configTime(0, 0, MY_NTP_SERVER);  // 0, 0 because we will use TZ in the next line
    setenv("TZ", MY_TZ, 1);            // Set environment variable with your time zone
    tzset();
  #else
    // ESP8266
    configTime(MY_TZ, MY_NTP_SERVER);    // --> for the ESP8266 only
  #endif

  // start network
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print ( "." );
  }
  Serial.println("\nWiFi connected");
  // by default, the NTP will be started after 60 secs
}

void loop() {
  showTime();
  delay(1000); // dirty delay
}