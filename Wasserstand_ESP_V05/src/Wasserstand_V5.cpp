/*
=============================================
Wasserstand_V6
V6.0
- both builds OK
- tests are to be done
=============================================
Wasserstand_V5
V5.3
- runs OK on ESP32
- some bugs fixed 
  - crash with sending mail
  - crash because overload of string
next step: adapt to ESP 8266
=============================================
Wasserstand_V4

Transferred to Visual Studio Code

including following features:
- Wasserstand abfragen anhand von vier Relais Ausgängen
- Aktiven Bereich anzeigen
- Hintergrundfarbe abhängig von Warn- oder Alarmlevel
- Info auf WebSeite anzeigen
- Beim Wechsel auf einen gefährlichen Zustand und zurück werden Info Mails (Text-Mail) gesendet

-----------------------------
V4.5
- simulation of changing pegel
- code for longtime values added
-   - extra ring buffers
-   - procedure to display graph
- some optimizations to save RAM space
-   - generate strings for graph where it is needed
-   - return from subprogram releases this RAM space at return
-   - ringbuffer for time now holds epochtime (unsigned long); conversion into string is done at handleGraph procedure
-----------------------------
V4.4
- code reworked (delete unused variables, separat code into own files)
-----------------------------
V4.3
- Show actual value on main-page
- Show history of values on page ListFiltered
-----------------------------
V4.2
Display as graph
------------------------------
V4.1
analog functions added

next steps
  - show values via Web
  - 
------------------------------
V4.0
no changes in features. Compiles now without error and warning on vscode
functions are separated in several source files

------------------------------
V2.2
----
fixed issues:
- actual date/time is actually not evaluated to avoid crash when sending email --> this has to be fixed next (unset debug_crash to reactivate)

new features implemented
- HTML Mail
  - Hyperlink auf die Hauptseite

new features comming soon:
- HTML Mail mit Farbe
- Testmail per klick

known issues: OTA download not possible "not enouth space"
-----------------------------------------------------------


*/

/************************************ */
/* includes                           */
/************************************ */
#include <waterlevel_defines.h>

#if BOARDTYPE == ESP32
  #include <Arduino.h>

  #include <waterlevel.h>

  #include <ESP_Mail_Client.h>
  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>

// use time.h from Arduino.h 
#include "time.h"                   // for time() ctime()

#include <ArduinoOTA.h>   // OTA Upload via ArduinoIDE

  #ifdef DEBUG_PRINT_RAW
    #include <Wire.h>
    #include <Adafruit_Sensor.h>
    #include <Adafruit_ADS1X15.h>
  #endif

  #include "soc/soc.h"            // disable brownout detector
  #include "soc/rtc_cntl_reg.h"   // disable brownout detector

  WebServer server(80);

  #include <server.h>
  #include <timeserver.h>
  #include <NoiascaCurrentLoop.h>   // library for analog measurement
  #include <EvaluateSensor.h>


  #include <ProjClient.h>

#else
  #include <Arduino.h>

  #include <waterlevel.h>

  #include <ESP_Mail_Client.h>
  #include <ESP8266mDNS.h>  // Bonjour/multicast DNS, finds the device on network by name
  //#include <NTPClient.h>    // get time from timeserver
  #include <ArduinoOTA.h>        // OTA Upload via ArduinoIDE


  // use time.h from Arduino.h 
  #include "time.h"                   // for time() ctime()

  #include <server.h>
  #include <timeserver.h>
  #include <NoiascaCurrentLoop.h>   // library for analog measurement
  #include <EvaluateSensor.h>

  #include <ProjClient.h>

#endif


extern CurrentLoopSensor currentLoopSensor();


#include <ProjClient.h>


// definitions for analog-digital conversion
#if MyUSE_ADC == ADS1115
   TwoWire I2CSensors = TwoWire(0);
   Adafruit_ADS1115 ads;
   int16_t adc0;
#endif

/********************************************************************
         Globals - Variables and constants
 ********************************************************************/

unsigned long seconds_since_startup = 0;      // current second since startup

unsigned long previousMillis = 0;             // used to determine intervall of ADC measurement
unsigned long longtermPreviousMillis = 0;     //
unsigned long millisNow      = 0;  

const uint16_t ajaxIntervall = 5;             // intervall for AJAX or fetch API call of website in seconds
uint32_t clientPreviousSs = 0;                // - clientIntervall;  // last second when data was sent to server

const uint16_t clientIntervall = 0;                      // intervall to send data to a server in seconds. Set to 0 if you don't want to send data
const char *sendHttpTo = "http://192.168.178.153/d.php"; // the module will send information to that server/resource. Use an URI or an IP address

int alarmState    = 0;    // shows the actual water level
int alarmStateOld = 0; // previous value of alarmState
bool executeSendMail = false;

int debugLevelSwitches_old = 0;

//* ============================================================= */
/* Definition for Send-Mail                                      */

/* settings for GMAIL */
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT esp_mail_smtp_port_587 // port 465 is not available for Outlook.com

/* The log in credentials */
#define AUTHOR_NAME "Pegelstand Zehentner"
#define AUTHOR_EMAIL "georgzehentneresp@gmail.com"
#define AUTHOR_PASSWORD "lwecoyvlkmordnly"

/* Recipient email address */
#ifdef isLiveSystem
  #define RECIPIENT_EMAIL "gzehentner@web.de"
#else
  #define RECIPIENT_EMAIL "gzehentner@t-online.de"
#endif

/* Declare the global used SMTPSession object for SMTP transport */
SMTPSession smtp;

Session_Config config;

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);

#include "HeapStat.h"
HeapStat heapInfo;

extern char _end;
// GZE extern "C" char *sbrk(int i);
char *ramstart = (char *)0x20070000;
char *ramend = (char *)0x20088000;

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
/* ===========   prepare timeserver =================*/

/* Configuration of NTP */
#define MY_NTP_SERVER "at.pool.ntp.org"           
#define MY_TZ "CET-1CEST,M3.5.0/02,M10.5.0/03"  


/* Globals */
time_t epochTime;                         // this are the seconds since Epoch (1970) - UTC

// declare global variables
String currentDate;   // hold the current date
String formattedTime; // hold the current time


void showTime() {
  tm tmL;
  time(&epochTime);                       // read the current time
  localtime_r(&epochTime, &tmL);           // update the structure tm with the current time
  Serial.print("year:");
  Serial.print(tmL.tm_year + 1900);  // years since 1900
  Serial.print("\tmonth:");
  Serial.print(tmL.tm_mon + 1);      // January = 0 (!)
  Serial.print("\tday:");
  Serial.print(tmL.tm_mday);         // day of month
  Serial.print("\thour:");
  Serial.print(tmL.tm_hour);         // hours since midnight  0-23
  Serial.print("\tmin:");
  Serial.print(tmL.tm_min);          // minutes after the hour  0-59
  Serial.print("\tsec:");
  Serial.print(tmL.tm_sec);          // seconds after the minute  0-61*
  Serial.print("\twday");
  Serial.print(tmL.tm_wday);         // days since Sunday 0-6
  if (tmL.tm_isdst == 1)             // Daylight Saving Time flag
    Serial.print("\tDST");
  else
    Serial.print("\tstandard");
  Serial.println();
}

void showTimeAlternative() {
  //time_t epochTime;                       // this are the seconds since Epoch (1970) - UTC
  tm tmL;                            // the structure tm holds time information in a more convenient way
  time(&epochTime);                       // read the current time
  localtime_r(&epochTime, &tmL);           // update the structure tm with the current time
  char buffer[42] {0};              // a buffer large enough to hold your output
  strftime (buffer, sizeof(buffer), "%H:%M", &tmL);  // for different formats see https://cplusplus.com/reference/ctime/strftime/
  Serial.println(buffer);
}

// void getFormattedDateAndTime()
// {
//   //time_t epochTime;                       // this are the seconds since Epoch (1970) - UTC
//   tm tmL;                           // the structure tm holds time information in a more convenient way
//   time(&epochTime);                       // read the current time
//   localtime_r(&epochTime, &tmL);          // update the structure tm with the current time
  
//   char buffer[42] {0};              // a buffer large enough to hold your output
//   strftime (buffer, sizeof(buffer), "%H:%M:%S", &tmL);  // for different formats see https://cplusplus.com/reference/ctime/strftime/
//   formattedTime = buffer;

//   strftime (buffer, sizeof(buffer), "%F", &tmL);  // for different formats see https://cplusplus.com/reference/ctime/strftime/
//   currentDate = buffer;

// }

void getEpochTime(time_t &epochTime)
{
  time(&epochTime);                       // read the current time
}

void formatDateAndTime(String &formattedTime, String &formattedDate, time_t epochTime)
{
  //time_t epochTime;                       // this are the seconds since Epoch (1970) - UTC
  tm tmL;                           // the structure tm holds time information in a more convenient way
  localtime_r(&epochTime, &tmL);          // update the structure tm with the current time
  
  char buffer[42] {0};              // a buffer large enough to hold your output
  strftime (buffer, sizeof(buffer), "%H:%M:%S", &tmL);  // for different formats see https://cplusplus.com/reference/ctime/strftime/
  formattedTime = buffer;

  strftime (buffer, sizeof(buffer), "%F", &tmL);  // for different formats see https://cplusplus.com/reference/ctime/strftime/
  formattedDate = buffer;

}

WiFiUDP ntpUDP;

/* End Timeserver */

#ifndef CSS_MAINCOLOR
#define CSS_MAINCOLOR "#8A0829" // fallback if no CSS_MAINCOLOR was declared for the board
#endif


/*=================================================================*/
/*  Prepare analog out        */
/*=================================================================*/

// fade LED PIN (replace with LED_BUILTIN constant for built-in LED)
// #define LED_PIN            builtin_led

float dutycylce  = 0;              // how bright the LED is

float fadeAmount = 0.0001;         // how many m to fade the LED by
float pegel      = Level_AL/100.0; // waterlevel in m


unsigned long previousMillisMemoryStatePrint;
unsigned long WaitingTimeMemoryStatePrint = 1000;

unsigned long previousMillis_halfSecondAction;
unsigned long halfSecond;

/*****************************************************************************************************************
 *****************************************************************************************************************
         S E T U P
 *****************************************************************************************************************
 *****************************************************************************************************************/

void setup(void) {

  #if BOARDTYPE == ESP32
     WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector
  #endif 

  pinMode(builtin_led, OUTPUT);

  /*=================================================================*/
  /* setup serial  and connect to WLAN */
  Serial.begin(115200);
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

  #if BOARDTYPE == ESP32

    if (MDNS.begin("esp32")) {
      Serial.println("MDNS responder started");
    }
    #else
    
    if (MDNS.begin("esp8266")) {
      Serial.println(F("MDNS responder started"));
      }
    #endif
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


  #if BOARDTYPE == ESP32
    // ESP32 seems to be a little more complex:
    configTime(0, 0, MY_NTP_SERVER);  // 0, 0 because we will use TZ in the next line
    setenv("TZ", MY_TZ, 1);            // Set environment variable with your time zone
    tzset();
  #else
    // ESP8266
    configTime(MY_TZ, MY_NTP_SERVER);    // --> for the ESP8266 only
  #endif


  /*=================================================================*/
  /* Prepare WaterLevel Application */

  // prepare relais input / output

  pinMode(GPin_AHH, INPUT_PULLUP);
  pinMode(GPin_AH,  INPUT_PULLUP);
  pinMode(GPin_AL,  INPUT_PULLUP);



  // for ESP32 we no longer use digital output for GND, but ESP32-GND
  #if (BOARDTYPE == ESP8266)
    pinMode(GPout_GND, OUTPUT);

    digitalWrite(GPout_GND, 0);
  #endif

  
  /* ----End Setup WaterLevel ------------------------------------------ */
  /*=================================================================*/

  /*=================================================================*/
  /* Setup WebServer and start*/

  // define the pages and other content for the webserver
  server.on("/", handlePage);      // send root page
  server.on("/0.htm", handlePage); // a request can reuse another handler
  server.on("/graph.htm", handleGraph);
  server.on("/longterm_graph.htm", handleLongtermGraph);
  server.on("/filtered.htm",handleListFiltered);

  server.on("/f.css", handleCss); // a stylesheet
  server.on("/j.js", handleJs);   // javscript based on fetch API to update the page
  // server.on("/j.js",  handleAjax);             // a javascript to handle AJAX/JSON update of the page  https://werner.rothschopf.net/201809_arduino_esp8266_server_client_2_ajax.htm
  server.on("/json", handleJson);    // send data in JSON format
  server.on("/c.php", handleCommand);            // process commands
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
  beginCurrentLoopSensor();
  
  Serial.print("is_life_system: ");
  Serial.println(isLiveSystem);
  
  #if MyUSE_ADC == ADS1115
    /*==================================================================*/
    // Prepare analog output
    //  pinMode(LED_PIN, OUTPUT);

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

    // PSRAM?
    log_d("Total heap: %d", ESP.getHeapSize());
    log_d("Free heap: %d", ESP.getFreeHeap());
    log_d("Total PSRAM: %d", ESP.getPsramSize());
    log_d("Free PSRAM: %d", ESP.getFreePsram());
  #endif

}
  /*==================================================================*/
  

/*****************************************************************************************************************
 *****************************************************************************************************************
         M A I N L O O P
 *****************************************************************************************************************
 *****************************************************************************************************************/


void loop(void) {

  // actions triggered every 500 ms
  if (millis() - previousMillis_halfSecondAction > halfSecond)
  {

    /*=================================================================*/
    /*  code for getting time from NTP       */
    getEpochTime(epochTime);
    formatDateAndTime(formattedTime, currentDate,epochTime);
    /* End getting time and date */
    
    previousMillis_halfSecondAction = millis();
  }


  delay(2);//allow the cpu to switch to other tasks

  if (debugLevelSwitches != debugLevelSwitches_old) {
    if (debugLevelSwitches) {
      pinMode(GPin_AHH, OUTPUT);
      pinMode(GPin_AH,  OUTPUT);
      pinMode(GPin_AL,  OUTPUT);
      
      digitalWrite(GPin_AHH, HIGH);
      digitalWrite(GPin_AH , HIGH);
      digitalWrite(GPin_AL , HIGH);
      Serial.println("debug level enabled");
    } else {
      pinMode(GPin_AHH, INPUT_PULLUP);
      pinMode(GPin_AH,  INPUT_PULLUP);
      pinMode(GPin_AL, INPUT_PULLUP);
      Serial.println("debug level disabled");
    }
  }
  debugLevelSwitches_old = debugLevelSwitches;


  /*=================================================================*/
  /* WebClient */

  seconds_since_startup = millis() / 1000;
  if (clientIntervall > 0 && (seconds_since_startup - clientPreviousSs) >= clientIntervall)
  {
    sendPost();
    clientPreviousSs = seconds_since_startup;
  }
  server.handleClient();


  /*=================================================================*/
  /* Over the Air UPdate */
  ArduinoOTA.handle(); // OTA Upload via ArduinoIDE

  /*=================================================================*/
  /* evaluate water level */
  /*=================================================================*/

  // Get waterlevel out of Current
  Current2Waterlevel();

  SetAlarmState_from_relais();

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

    // htmlMsg already set by Evaluate Sensor
    message.html.content = htmlMsg;
    message.text.content = F("");

    Serial.println();
    Serial.println(F("Sending Email..."));

    if (!smtp.isLoggedIn())
    {
      /* Set the TCP response read timeout in seconds */
      smtp.setTCPTimeout(10);
      
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
    // change the dutycylce for next time through the loop:
    pegel = pegel + fadeAmount;

    // reverse the direction of the fading at the ends of the fade:
    if (pegel <= Level_ALL/100.0-0.05 || pegel >= Level_AHH/100.0+0.05) {
      fadeAmount = -fadeAmount;
    }
  #endif // SIM_FADING_LEVEL
  
  #ifdef DEBUG_PRINT_RAW
    /*=================================================================*/
    // read analog value via I2C for debug
    // not used in live system
    //===========================================
    const int loc_maxAdc_value = 0x7FFF;
    float voltage=0.0;

    Serial.println("=================================");
    adc0 = ads.readADC_Differential_0_1();
    Serial.print("Analog input pin 0: "); Serial.println(adc0);

  // voltage = ads.computeVolts(adc0);   
  // Serial.print("Voltage: "); Serial.println(voltage);

   #endif // DEBUG_PRINT_RAW


  
  #ifdef DEBUG_PRINT_HEAP
    if (millis() - previousMillisMemoryStatePrint > WaitingTimeMemoryStatePrint)
    {
      Serial.println(formattedTime);
      heapInfo.collect();
      heapInfo.print();
      
      previousMillisMemoryStatePrint = millis();
    }
  #endif

  // set a delay to avoid ESP is busy all the time
  delay(10);

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

