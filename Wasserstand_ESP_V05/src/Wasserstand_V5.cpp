/*
   Copyright (c) 2015, Majenko Technologies
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

 * * Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.

 * * Redistributions in binary form must reproduce the above copyright notice, this
     list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.

 * * Neither the name of Majenko Technologies nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <Arduino.h>
#include <ESP_Mail_Client.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#include <NTPClient.h>    // get time from timeserver

#ifdef DEBUG_PRINT_RAW
  #include <Wire.h>
  #include <Adafruit_Sensor.h>
  #include <Adafruit_ADS1X15.h>
#endif

#include <ArduinoOTA.h>   // OTA Upload via ArduinoIDE

#include <server.h>
#include <timeserver.h>
#include <waterlevel_defines.h>
#include <waterlevel.h>
#include <NoiascaCurrentLoop.h>   // library for analog measurement
#include <EvaluateSensor.h>

extern CurrentLoopSensor currentLoopSensor();

WebServer server(80);

// definitions for analog-digital conversion
#if BOARDTYPE == ESP32
   TwoWire I2CSensors = TwoWire(0);
   Adafruit_ADS1115 ads;
   int16_t adc0;
#endif

/********************************************************************
         Globals - Variables and constants
 ********************************************************************/

unsigned long seconds_since_startup = 0;      // current second since startup

unsigned long previousMillis = 0;             // used to determine intervall of ADC measurement
unsigned long millisNow      = 0;  

const uint16_t ajaxIntervall = 5;             // intervall for AJAX or fetch API call of website in seconds
uint32_t clientPreviousSs = 0;                // - clientIntervall;  // last second when data was sent to server

const uint16_t clientIntervall = 0;                      // intervall to send data to a server in seconds. Set to 0 if you don't want to send data
const char *sendHttpTo = "http://192.168.178.153/d.php"; // the module will send information to that server/resource. Use an URI or an IP address

// int inByte = 0;
// int incomingByte = 0; // for incoming serial data

int alarmState;    // shows the actual water level
int alarmStateOld; // previous value of alarmState
bool executeSendMail = false;

// level switches coming from external box
int val_AHH;
int val_AH;
int val_AL;
int val_ALL;

/* ============================================================= */
/* Definition for Send-Mail                                      */

/* settings for GMAIL */
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT esp_mail_smtp_port_587 // port 465 is not available for Outlook.com

/* The log in credentials */
#define AUTHOR_NAME "Pegelstand Zehentner"
#define AUTHOR_EMAIL "georgzehentneresp@gmail.com"
#define AUTHOR_PASSWORD "lwecoyvlkmordnly"

/* Recipient email address */
#define RECIPIENT_EMAIL "gzehentner@web.de"

/* Declare the global used SMTPSession object for SMTP transport */
SMTPSession smtp;

Session_Config config;

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);

#include "HeapStat.h"
HeapStat heapInfo;

// const char rootCACert[] PROGMEM = "-----BEGIN CERTIFICATE-----\n"
//                                   "-----END CERTIFICATE-----\n";
/* END Definition for Send-Mail                                      */
/* ============================================================= */

/* *******************************************************************
         other settings / weitere Einstellungen für den Anwender
 ********************************************************************/

#ifndef STASSID                // either use an external .h file containing STASSID and STAPSK or ...
#define STASSID "Zehentner"    // ... modify these line to your SSID
#define STAPSK "ElisabethScho" // ... and set your WIFI password
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

/*=================================================================*/
/* Variables to connect to timeserver   */
/* Define NTP Client to get time */

String currentDate;   // hold the current date
String formattedTime; // hold the current time

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

/* End Timeserver */

#ifndef CSS_MAINCOLOR
#define CSS_MAINCOLOR "#8A0829" // fallback if no CSS_MAINCOLOR was declared for the board
#endif


/*=================================================================*/
/*  Prepare analog out        */
/*=================================================================*/

// fade LED PIN (replace with LED_BUILTIN constant for built-in LED)
#define LED_PIN            builtin_led

float dutycylce  = 0;              // how bright the LED is
// GZE
float fadeAmount = 0;
// float fadeAmount = 0.0001;         // how many m to fade the LED by
float pegel      = Level_AL/100.0; // waterlevel in m

/*****************************************************************************************************************
 *****************************************************************************************************************
         S E T U P
 *****************************************************************************************************************
 *****************************************************************************************************************/

void setup(void) {
  pinMode(builtin_led, OUTPUT);
//  digitalWrite(led, 0);

 /*=================================================================*/
  /* setup serial  and connect to WLAN */
  Serial.begin(9600);
  Serial.println(F("\n" TXT_BOARDNAME "\nVersion: " VERSION " Board " TXT_BOARDID " "));
  Serial.print(__DATE__);
  Serial.print(F(" "));
  Serial.println(__TIME__);

  // Connect to WIFI
  char myhostname[8] = {"esp"};
  strcat(myhostname, TXT_BOARDID);
  WiFi.hostname(myhostname);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  /*=================================================================*/
  // /* Prepare SendMail */

  MailClient.networkReconnect(true);
  smtp.debug(1);

  smtp.callback(smtpCallback);

  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;

  config.login.user_domain = F("127.0.0.1");

  /*
  Set the NTP config time
  For times east of the Prime Meridian use 0-12
  For times west of the Prime Meridian add 12 to the offset.
  Ex. American/Denver GMT would be -6. 6 + 12 = 18
  See https://en.wikipedia.org/wiki/Time_zone for a list of the GMT/UTC timezone offsets
  */
  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 1;
  config.time.day_light_offset = 1;

  /*=================================================================*/
  /* Prepare WaterLevel Application */

  // prepare relais input / output

  pinMode(GPin_AHH, INPUT_PULLUP);
  pinMode(GPin_AH, INPUT_PULLUP);
  pinMode(GPin_AL, INPUT_PULLUP);
  pinMode(GPin_ALL, INPUT_PULLUP);
  
  // we no longer use digital output for GND, but ESP32-GND
  // pinMode(GPout_GND, OUTPUT);
  // digitalWrite(GPout_GND, 0);

  /* ----End Setup WaterLevel ------------------------------------------ */
  /*=================================================================*/

  /*=================================================================*/
  /* Setup WebServer and start*/

  // define the pages and other content for the webserver
  server.on("/", handlePage);      // send root page
  server.on("/0.htm", handlePage); // a request can reuse another handler
  server.on("/graph.htm", handleGraph);
  server.on("/filtered.htm",handleListFiltered);

  server.on("/f.css", handleCss); // a stylesheet
  server.on("/j.js", handleJs);   // javscript based on fetch API to update the page
  // server.on("/j.js",  handleAjax);             // a javascript to handle AJAX/JSON update of the page  https://werner.rothschopf.net/201809_arduino_esp8266_server_client_2_ajax.htm
  server.on("/json", handleJson);    // send data in JSON format
                                     //  server.on("/c.php", handleCommand);            // process commands
                                     //  server.on("/favicon.ico", handle204);          // process commands
  server.onNotFound(handleNotFound); // show a typical HTTP Error 404 page

  // the next two handlers are necessary to receive and show data from another module
  //  server.on("/d.php", handleData);               // receives data from another module
  //  server.on("/r.htm", handlePageR);              // show data as received from the remote module


  // following settings are coming from AdvancedWebServer
  // server.on("/test.svg", drawGraph);
  // server.on("/inline", []() {
  //   server.send(200, "text/plain", "this works as well");
  // });

  server.begin(); // start the webserver
  Serial.println(F("HTTP server started"));

  /*=================================================================*/
  /* IDE OTA */
  ArduinoOTA.setHostname(myhostname); // give a name to your ESP for the Arduino IDE
  ArduinoOTA.begin();                 // OTA Upload via ArduinoIDE https://arduino-esp8266.readthedocs.io/en/latest/ota_updates/readme.html



  /*=================================================================*/
  /* Initialize a NTPClient to get time */

  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(3600);

  /*=================================================================*/
  beginCurrentLoopSensor();
  
  /*==================================================================*/
  // Prepare analog output
    pinMode(LED_PIN, OUTPUT);

  /*==================================================================*/
  // prepare I2C interface
  I2CSensors.begin(I2C_SDA, I2C_SCL, 100000);
  
  /*==================================================================*/
  // prepare analog read
  // ADS 1115 (0x48 .. 0x4B will be the address)
  if (!ads.begin(0x48, &I2CSensors))
  {
    Serial.println("Couldn't Find ADS 1115");
    while (1)
      ;
  }
  else
  {
    Serial.println("ADS 1115 Found");
    ads.setGain(GAIN_ONE);
    Serial.print("Gain: ");
  Serial.println(ads.getGain());
    }
  

}
  /*==================================================================*/
  

/*****************************************************************************************************************
 *****************************************************************************************************************
         M A I N L O O P
 *****************************************************************************************************************
 *****************************************************************************************************************/


void loop(void) {

  delay(2);//allow the cpu to switch to other tasks

  /*=================================================================*/
  /* WebClient (not used yet)*/

  seconds_since_startup = millis() / 1000;
  if (clientIntervall > 0 && (seconds_since_startup - clientPreviousSs) >= clientIntervall)
  {
    //   sendPost();
    clientPreviousSs = seconds_since_startup;
  }
  server.handleClient();


  /*=================================================================*/
  /* Over the Air UPdate */
  ArduinoOTA.handle(); // OTA Upload via ArduinoIDE

    /*=================================================================*/
  /* WebClient (not used yet)*/

  seconds_since_startup = millis() / 1000;
  if (clientIntervall > 0 && (seconds_since_startup - clientPreviousSs) >= clientIntervall)
  {
    //   sendPost();
    clientPreviousSs = seconds_since_startup;
  }
  server.handleClient();

  /*=================================================================*/
  /* evaluate water level */
  /*=================================================================*/

  // Get waterlevel out of Current
  Current2Waterlevel();

  SetAlarmState_from_relais();

  /*=================================================================*/
  /*  code for getting time from NTP       */
  timeClient.update();

  time_t epochTime = timeClient.getEpochTime();

  formattedTime = timeClient.getFormattedTime();

  // Get a time structure
  struct tm *ptm = gmtime((time_t *)&epochTime);

  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon + 1;
  int currentYear = ptm->tm_year + 1900;

  // Print complete date:
  currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);


  /* End getting time and date */


  /*=================================================================*/
  /* Send Email reusing session   */
  /*=================================================================*/

  if (executeSendMail)
  {
    executeSendMail = false;

    SMTP_Message message;

    message.sender.name = F("Pegel Zehentner");
    message.sender.email = AUTHOR_EMAIL;
    message.subject = subject;

    message.addRecipient(F("Schorsch"), RECIPIENT_EMAIL);

    // htmlMsg already set by Waterlevel
    message.html.content = htmlMsg;
    message.text.content = F("");

    Serial.println();
    Serial.println(F("Sending Email..."));

    if (!smtp.isLoggedIn())
    {
      /* Set the TCP response read timeout in seconds */
      // smtp.setTCPTimeout(10);

      if (!smtp.connect(&config))
      {
        MailClient.printf("Connection error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
        goto exit;
      }

      if (!smtp.isLoggedIn())
      {
        Serial.println(F("Error, Not yet logged in."));
        goto exit;
      }
      else
      {
        if (smtp.isAuthenticated())
          Serial.println(F("Successfully logged in."));
        else
          Serial.println(F("Connected with no Auth."));
      }
    }

    if (!MailClient.sendMail(&smtp, &message, false))
      MailClient.printf("Error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());

  exit:

    heapInfo.collect();
    heapInfo.print();

    /*=END Send_Reuse_Session =====================================*/
  }
  
  /*===========================================================
    simulate changing waterlevel
   */
  #ifdef SIM_FADING_LEVEL
  dutycylce = Waterlevel2dutyCycle(pegel);

  analogWrite(LED_PIN, dutycylce);

  // change the dutycylce for next time through the loop:
  pegel = pegel + fadeAmount;

  // reverse the direction of the fading at the ends of the fade:
  if (pegel <= Level_ALL/100.0-0.05 || pegel >= Level_AHH/100.0+0.05) {
    fadeAmount = -fadeAmount;
  }
  // Serial.println("===================");
  // Serial.print("pegel:     "); Serial.println(pegel);
  //Serial.print("dutycylce: "); Serial.println(dutycylce);
  #endif // SIM_FADING_LEVEL
  
  #ifdef DEBUG_PRINT_RAW
  /*=================================================================*/
  // read analog value via I2C for debug
  // not used in live system
  //===========================================
  const int loc_maxAdc_value = 0x7FFF;
  float voltage=0.0;

  Serial.println("=================================");
  adc0 = ads.readADC_SingleEnded(0);
  Serial.print("Analog input pin 0: "); Serial.println(adc0);

  voltage = ads.computeVolts(adc0);   
  Serial.print("Voltage: "); Serial.println(voltage);

  #endif // DEBUG_PRINT_RAW

/*==================================================================================================================================*/
} // end void loop()
/*==================================================================================================================================
  ==================================================================================================================================*/

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status)
{

  Serial.println(status.info());

  if (status.success())
  {

    Serial.println(F("----------------"));
    MailClient.printf("Message sent success: %d\n", status.completedCount());
    MailClient.printf("Message sent failed: %d\n", status.failedCount());
    Serial.println(F("----------------\n"));

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      SMTP_Result result = smtp.sendingResult.getItem(i);

      MailClient.printf("Message No: %d\n", i + 1);
      MailClient.printf("Status: %s\n", result.completed ? "success" : "failed");
      MailClient.printf("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      MailClient.printf("Recipient: %s\n", result.recipients.c_str());
      MailClient.printf("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    smtp.sendingResult.clear();
  }
}

