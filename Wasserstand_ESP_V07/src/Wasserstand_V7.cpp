/*
Wasserstand_V7
Renamed to new version, because of changes in graphs ..
=============================================
Wasserstand_V5
V6.9
- Pumpensteuerung implementiert
  - sechster Alarmlevel, der gleich zwei Pumpenstartet
  - Betriebszeit der Pumpen in Datei gespeichert
- Messdaten zur grafischen Aufbereitung werden zur bplaced-Seite geschickt
  - schicken der Daten
  - ablegen in Datenbank
  - lesen aus Datenbank und einfache Grafik
=============================================
Wasserstand_V5
V6.4
- Hysteresis added at alarmStateLevel
=============================================
Wasserstand_V5
V6.3
- System running and tested
=============================================
Wasserstand_V5
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
#include <myWiFiLib.h>

#include <ESP_Mail_Client.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

// use time.h from Arduino.h
#include "time.h" // for time() ctime()

#include "soc/soc.h"          // disable brownout detector
#include "soc/rtc_cntl_reg.h" // disable brownout detector

WebServer server(80);

#include <server.h>
#include <timeserver.h>
#include <NoiascaCurrentLoop.h> // library for analog measurement
#include <EvaluateSensor.h>

#include <ProjClient.h>

#include <pumpControl.h>

#include <MyLittleFSLib.h>

#include <SerialLink.h>

#else // ESP8266
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include <waterlevel.h>

#include <ESP_Mail_Client.h>
#include <ESP8266mDNS.h> // Bonjour/multicast DNS, finds the device on network by name

#include <server.h>
#include <timeserver.h>
#include <NoiascaCurrentLoop.h> // library for analog measurement
#include <EvaluateSensor.h>

#include <ProjClient.h>
#include <LittleFS.h>

#include <pumpControl.h>

#include <MyLittleFSLib.h>
#endif

// #include <TelnetStream.h>
// #include <debugPrint.h>

#include <ElegantOTA.h>
#include "esp_task_wdt.h" // Include ESP32 Watchdog Timer library

#if MyUSE_ADC == ADS1115_ADC
#if BOARDTYPE == ESP32
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADS1X15.h>
TwoWire I2CSensors = TwoWire(0);
Adafruit_ADS1115 ads;
int16_t adc0;
#else
#include <Adafruit_ADS1X15.h>
#include <Wire.h>

Adafruit_ADS1115 ads;

int16_t adc0;
#endif
#endif

extern CurrentLoopSensor currentLoopSensor();

#include <ProjClient.h>

/********************************************************************
         Globals - Variables and constants
 ********************************************************************/

// int testGlobal = 0;      // global variable to test the cyclic print
// int op1timeGlobal = 0;   // global variable to hold the operation time of pump 1
// int op2timeGlobal = 0;   // global variable to hold the operation time of pump 2
// int pumpA_op_global = 0; // global variable to hold the operation state of pump A
// int pumpB_op_global = 0; // global variable to hold the operation state of pump
// int pump1_op_global = 0; // global variable to hold the operation state of pump 1
// int pump2_op_global = 0; // global variable to hold the operation state of pump 2

unsigned long seconds_since_startup = 0; // current second since startup

unsigned long previousMillis = 0; // used to determine intervall of ADC measurement
unsigned long millisNow = 0;

// measure maxLoopRuntime
unsigned long maxLoopRuntime = 0; // maximum runtime for one man loop
unsigned long actLoopRuntime = 0;
unsigned long previousMillisLoopRuntime = 0; // last timestamp for loop
unsigned long millisNowLoopRuntime = 0;      // actual timestamp

unsigned long dailyPumpRunTime = DAILY_PUMP_RUN_TIME;

const uint16_t ajaxIntervall = 5; // intervall for AJAX or fetch API call of website in seconds
uint32_t clientPreviousSs = 0;    // - clientIntervall;  // last second when data was sent to server

#if isLiveSystem == 1                                   /* dont send */
const uint16_t clientIntervall = CLIENT_INTERVALL_LIFE; // intervall to send data to a server in seconds. Set to 0 if you don't want to send data
                                                        // live systen shall not send anything for now
#else
const uint16_t clientIntervall = CLIENT_INTERVALL_DEV; // intervall to send data to a server in seconds. Set to 0 if you don't want to send data
#endif

// select http url to send to:
//  - send to Bplaced or an internal device
//  - select if it is a live system or development
#if isLiveSystem == 1
const char *sendHttpTo = "https://zehentner.bplaced.net/Wasserstand/live/data_sql.php"; // the module will send information to that server/resource. Use an URI or an IP address
#else
const char *sendHttpTo = "https://zehentner.bplaced.net/Wasserstand/dev/data_sql.php"; // the module will send information to that server/resource. Use an URI or an IP address
#endif

int alarmStateRelais = 0;    // actual state of alarm derived from relais
int alarmStateRelaisOld = 0; // previous value of alarmStateRelais
int alarmStateLevel = 0;     // actual state of alarm derived from measured water level
int alarmStateLevelOld = 0;  // previous value of alarmStateLevel
int alarmState = 0;          // worst case value of both alarmState
                             //   5 : pump running
                             //   4 : severe warning
                             //   3 : normal high
                             //   2 : normal
                             //   1 : normal low
int alarmStateOld = 0;       // previous value of alarmState
bool executeSendMail = false;

int debugLevelSwitches_old = 0;

// SerialLink-Objekt für Kommunikation mit zweitem ESP32
#if BOARDTYPE == ESP32
SerialLink serialLink;
unsigned long lastSerialLinkSend = 0;
const uint16_t serialLinkInterval = 5; // Sende-Intervall in Sekunden
#endif
String receivedString = "";

char input[64];
bool inputReadingCompleted = false;

long lastMillis = 0;
int timeTickForCyclicPrint = 0;

int doPrint = 0;
int value = 0;

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
#define RECIPIENT_EMAIL_LIVE "gzehentner@web.de"
#define RECIPIENT_EMAIL_DEV "gzehentner@t-online.de"

#if isLiveSystem == 1
#define RECIPIENT_EMAIL RECIPIENT_EMAIL_LIVE
#else
#define RECIPIENT_EMAIL RECIPIENT_EMAIL_DEV
#endif

/* Declare the global used SMTPSession object for SMTP transport */
SMTPSession smtp;

Session_Config config;

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);

#include "HeapStat.h"
HeapStat heapInfo;

// GZE: I dont know what this par of code does
extern char _end;
#if BOARDTYPE == ESP8266
extern "C" char *sbrk(int i);
#endif

char *ramstart = (char *)0x20070000;
char *ramend = (char *)0x20088000;

// const char rootCACert[] PROGMEM = "-----BEGIN CERTIFICATE-----\n"
//                                   "-----END CERTIFICATE-----\n";
/* END Definition for Send-Mail                                      */
/* ============================================================= */

/* *******************************************************************
         other settings / weitere Einstellungen für den Anwender
 ********************************************************************/
// WIFI Settings moved to myWiFiLib.h
String phoneNumber = "+491607547424";
String apiKey = "6878208";

WiFiUDP ntpUDP;

#ifndef CSS_MAINCOLOR
#define CSS_MAINCOLOR "#8A0829" // fallback if no CSS_MAINCOLOR was declared for the board
#endif

/*=================================================================*/
/*  Prepare analog out        */
/*=================================================================*/

//--------------- fading LED not used for simulation
//
// fade LED PIN (replace with LED_BUILTIN constant for built-in LED)
// #define LED_PIN            builtin_led

// float dutycylce  = 0;              // how bright the LED is

// float fadeAmount = 0.0001;         // how many m to fade the LED by

//--------------------------------------------------------------------

unsigned long previousMillisCyclicPrint;
unsigned long WaitingTimeCyclicPrint = 1000;

unsigned long previousMillis_halfSecondAction;
unsigned long halfSecond;

void flushInput()
{
  for (int i = 0; i < 64; i++)
  {
    input[i] = {};
  }
}

void debugPrintCyclic(int index, long WaitingTimeCyclicPrint, String printText, int newline, long printValue)
{
  static long previousMillisCyclicPrint[20] = {0};

  // print some debug information cyclic
  if (millis() - previousMillisCyclicPrint[index] > WaitingTimeCyclicPrint)
  {
    previousMillisCyclicPrint[index] = millis();

    Serial.print(printText);
    Serial.print(printValue);
    if (newline)
    {
      Serial.println();
    }
    else
    {
      Serial.print(" ");
    }
  }
} // end of debugPrintCyclic

MeasureRuntime measureSensor = {0, 0, 0};
MeasureRuntime measureLoopOthers = {0, 0, 0};
MeasureRuntime measureLoopAll = {0, 0, 0};

void measureRuntimeStart(MeasureRuntime &measureRuntime)
{
  measureRuntime.startTime = millis();
}

void measureRuntimeEnd(MeasureRuntime &measureRuntime)
{
  measureRuntime.runTime = millis() - measureRuntime.startTime;

  if (measureRuntime.runTime > measureRuntime.maxDelta)
  {
    measureRuntime.maxDelta = measureRuntime.runTime;
  }
}

//===================================================================*/
// variables for toplevel
PumpStatus pumpStatus = {0, 0, 0};      // create a PumpStatus object to manage pump states
PumpControl pumpControl = {0, 0, 0, 0, 0, 0, 0, 0, 0}; // create a PumpControl object to manage pump operations
int triggerCheckPump;
//===================================================================*/

/*****************************************************************************************************************
 *****************************************************************************************************************
         S E T U P
 *****************************************************************************************************************
 *****************************************************************************************************************/

void setup(void)
{

#if BOARDTYPE == ESP32
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector
#endif

  pinMode(builtin_led, OUTPUT);

  /*=================================================================*/
  /* =================  setup serial     */
  Serial.begin(115200);

  // wait for serial to come online
  while (!Serial)
    ;
  Serial.println("Serial is ready to accept input");
  //------------------------

  
  /*=================================================================*/
  // print board information

  
  
  Serial.println(F("\n" TXT_BOARDNAME "\nVersion: " VERSION " Board " TXT_BOARDID " "));
  Serial.print(__DATE__);
  Serial.print(F(" "));
  Serial.println(__TIME__);

  // Check the reason for the last reset
  esp_reset_reason_t resetReason = esp_reset_reason();
  if (resetReason == ESP_RST_TASK_WDT || resetReason == ESP_RST_WDT)
  {
    Serial.println("Reboot caused by Watchdog Timer!");
  }
  else
  {
    Serial.print("Reboot reason: ");
    Serial.println(resetReason);
  }

  /*=================  Connect to WIFI */
  connectWifi();
  
#if BOARDTYPE == ESP32

  if (MDNS.begin("esp32"))
  {
    Serial.println("MDNS responder started");
  }
#else

  if (MDNS.begin("esp8266"))
  {
    Serial.println(F("MDNS responder started"));
  }
#endif
  /*=================================================================*/
  /*====================   Prepare SendMail */

  MailClient.networkReconnect(true);
  smtp.debug(0);

  smtp.callback(smtpCallback);

  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;

  config.login.user_domain = F("127.0.0.1");

  /*=================================================================*/
  /*====================   Prepare connection to timeserver */

#if BOARDTYPE == ESP32
  // ESP32
  configTime(0, 0, MY_NTP_SERVER); // 0, 0 because we will use TZ in the next line
  setenv("TZ", MY_TZ, 1);          // Set environment variable with your time zone
  tzset();
#else
  // ESP8266
  configTime(MY_TZ, MY_NTP_SERVER); // --> for the ESP8266 only
#endif

  /*=================================================================*/
  /* Prepare WaterLevel Application */

  // prepare relais input / output

  pinMode(GPin_AHHH, INPUT_PULLUP);
  pinMode(GPin_AHH, INPUT_PULLUP);
  pinMode(GPin_AH, INPUT_PULLUP);
  pinMode(GPin_AL, INPUT_PULLUP);

  digitalWrite(GPout_pump1, 0);
  digitalWrite(GPout_pump2, 0);
  pinMode(GPout_pump1, OUTPUT);
  pinMode(GPout_pump2, OUTPUT);

  /* ----End Setup WaterLevel ------------------------------------------ */
  /*=================================================================*/

  /*=================================================================*/
  /*=================================================================*/
  /* Setup WebServer and start*/
  /*=================================================================*/

  // define the pages and other content for the webserver
  server.on("/", handlePage);           // send root page
  server.on("/0.htm", handlePage);      // a request can reuse another handler
  server.on("/graph.htm", handleGraph); // display a chart with shortterm values
  server.on("/filtered.htm", handleListFiltered);

  server.on("/f.css", handleCss);     // a stylesheet
  server.on("/j.js", handleJs);       // javscript based on fetch API to update the page
  server.on("/json", handleJson);     // send data in JSON format
  server.on("/c.php", handleCommand); // process commands

  server.onNotFound(handleNotFound); // show a typical HTTP Error 404 page

  // the next two handlers are necessary to receive and show data from another module
  server.on("/d.php", handleData);  // receives data from another module
  server.on("/r.htm", handlePageR); // show data as received from the remote module

  server.on("/ota.htm", []()
            { server.send(200, "text/plain", "Hi! This is ElegantOTA Demo."); });

  ElegantOTA.begin(&server); // Start ElegantOTA
  server.begin();            // start the webserver
  Serial.println(F("HTTP server started"));
  /*=====================   end fo prepare webserver  ===============*/

  /*=================================================================*/
  beginCurrentLoopSensor();

  Serial.print("is_life_system: ");
  Serial.println(isLiveSystem);

#if MyUSE_ADC == ADS1115_ADC
  /*==================================================================*/
  // Prepare analog output
  //  pinMode(LED_PIN, OUTPUT);

#if BOARDTYPE == ESP32
  /*==================================================================*/
  // prepare I2C interface
  Serial.println("I2CSensors.begin");
  I2CSensors.begin(I2C_SDA, I2C_SCL, 100000);
#endif

/*==================================================================*/
// prepare analog read from ads1115 via I2C
// ADS 1115 (0x48 .. 0x4B will be the address)
#if BOARDTYPE == ESP8266
  Wire.begin(I2C_SDA, I2C_SCL);
#endif

// check if ADS is runnint
#if BOARDTYPE == ESP32
  // for ESP32
  Serial.println("ads.begin");
  if (!ads.begin(0x48, &I2CSensors))
#else
  // for ESP 8266
  if (!ads.begin())
#endif
  {
    Serial.println("Couldn't Find ADS 1115");
    // while  (1) // endless loop is not good, because ESP is not updatable by OTA
    ;
  }
  else
  {
    Serial.println("ADS 1115 Found");
    ads.setGain(GAIN_ONE);
    Serial.print("Gain: ");
    Serial.println(ads.getGain());
  }

#if BOARDTYPE == ESP32
  // PSRAM available?
  log_d("Total heap: %d", ESP.getHeapSize());
  log_d("Free heap: %d", ESP.getFreeHeap());
  log_d("Total PSRAM: %d", ESP.getPsramSize());
  log_d("Free PSRAM: %d", ESP.getFreePsram());
#endif
#endif

  /*=================================================================*/
  // SerialLink initialisieren (nur ESP32)
#if BOARDTYPE == ESP32
  Serial.println(F("Initializing SerialLink..."));
  serialLink.begin(9600, 4, 27); // 9600 baud, RX=GPIO4, TX=GPIO27 (GPIO17 belegt durch GPin_AL)
  Serial.println(F("SerialLink ready"));
#endif
  /*=================================================================*/

  //==========================================
  // prepare logfile
  bool fsOK = false;

  Serial.println("Mount LittleFS");

  if (!LittleFS.begin())
  {
    Serial.println("LittleFS mount failed, trying to format");

    if (!LittleFS.format())
    {
      Serial.println("LittleFS format failed");
    }
    else
    {

      Serial.println("format succeeded, try to mount agait");
      if (!LittleFS.begin())
      {
        Serial.println("mount failed again");
      }
      else
      {
        Serial.println("mount succeeded");
        fsOK = true;
      }
    }
  }
  else
  {
    fsOK = true;
  }

  if (fsOK)
  {

//=============================================================================================
// handling setup.ini
//=============================================================================================
#if deleteSetupFile == 0
    // deleting setupFile
    Serial.println("deleting setupFile");
    deleteFile("/setup.ini");
#else
    Serial.println("NOT!!! deleting setup.ini");
#endif

    getSetupIni(pumpStatus);
    // readFile("/setup.ini");
    Serial.println();
    Serial.print("pump1_operationTime : ");
    Serial.println(pumpStatus.pump1_operationTime);
    Serial.print("pump2_operationTime : ");
    Serial.println(pumpStatus.pump2_operationTime);
    Serial.print("linkPump            : ");
    Serial.println(pumpStatus.linkPump);

//=============================================================================================
// handling error lot
//=============================================================================================
#if deleteErrLog == 1
    // deleting setupFile
    Serial.println("deleting error.log");
    deleteFile("/error.log");
#else
    Serial.println("NOT!!! deleting error.log");
#endif
    if (!LittleFS.exists("/error.log"))
    {
      Serial.println("error.log doesnt exist; generating a new one");

      String errMessage = "";
      errMessage = currentDate;
      errMessage += " - ";
      errMessage += formattedTime;
      errMessage += " - ";
      errMessage += "init error-file\n";
      appendFile("/error.log", errMessage.c_str());
    }

    readFile("/error.log");
  }

  // listDir("/");

  // write reboot reason if not normal

  if ((resetReason != ESP_RST_POWERON) && (resetReason != ESP_RST_SW))
  {
    // write the reboot reason to the error log
    Serial.println("Writing reboot reason to error.log");

    // get current time and date
    getEpochTime(epochTime);
    formatDateAndTime(formattedTime, currentDate, epochTime);

    // write the reboot reason to the error log
    String errMessage = "";
    errMessage = currentDate;
    errMessage += " - ";
    errMessage += formattedTime;
    errMessage += " - ";
    errMessage += "Reboot reason: ";
    errMessage += resetReason;
    errMessage += "\n";
    appendFile("/error.log", errMessage.c_str());

  } // end of if resetReason != ESP_RST_POWERON

  // Initialize the Watchdog Timer
  esp_task_wdt_init(WDT_TIMEOUT, true);  // Enable panic so ESP32 resets
  esp_task_wdt_add(NULL);  // Add the current task (loop) to the Watchdog

}
  

/*****************************************************************************************************************
 *****************************************************************************************************************
         M A I N L O O P
 *****************************************************************************************************************
 *****************************************************************************************************************/

void loop(void)
{

  pumpControl.manualPumpControl = manualPumpControl;
  pumpControl.manPump1Enabled = manPump1Enabled;
  pumpControl.manPump2Enabled = manPump2Enabled;




  measureRuntimeEnd(measureLoopAll);
  measureRuntimeEnd(measureLoopOthers);

  measureRuntimeStart(measureLoopAll);

  // Reset the Watchdog Timer at the beginning of each loop iteration
  if (simTimeout == 1)
  {
    delay((WDT_TIMEOUT + 1) * 1000); // wait for WDT_TIMEOUT plus 1 second to simulate a timeout
    simTimeout = 0;                  // reset the timeout flag
  }
  esp_task_wdt_reset();

  // **************************************************************************************************
  // ===============================================================================
  // following actions are triggered every 500 ms
  if (millis() - previousMillis_halfSecondAction > halfSecond)
  {

    /*=================================================================*/
    /*  code for getting time from NTP       */
    getEpochTime(epochTime);
    formatDateAndTime(formattedTime, currentDate, epochTime);
    /* End getting time and date */

    previousMillis_halfSecondAction = millis();
  }
  // ===============================================================================
  // **************************************************************************************************

  // ============================================================================
  // In simulation mode:
  //       Simulate level and relais outputs
  //
  //  GZE shift to a separat file
  // ============================================================================
  if (debugLevelSwitches != debugLevelSwitches_old)
  {
    if (debugLevelSwitches == 1)
    {
      // set default values to last actual values, when debug is switched on
      Serial.print("val_AHHH: ");
      Serial.println(val_AHHH);
      Serial.print("val_AHH:  ");
      Serial.println(val_AHH);
      Serial.print("val_AH:   ");
      Serial.println(val_AH);
      Serial.print("val_AL:   ");
      Serial.println(val_AL);
      // simVal_AHHH = val_AHHH;
      simVal_AHH = val_AHH;
      simVal_AH = val_AH;
      simVal_AL = val_AL;
      pinMode(GPin_AHHH, OUTPUT);
      pinMode(GPin_AHH, OUTPUT);
      pinMode(GPin_AH, OUTPUT);
      pinMode(GPin_AL, OUTPUT);

      // digitalWrite(GPin_AHHH, simVal_AHHH);
      digitalWrite(GPin_AHH, simVal_AHH);
      digitalWrite(GPin_AH, simVal_AH);
      digitalWrite(GPin_AL, simVal_AL);
      Serial.println("debug level enabled");
    }
    else
    {
      pinMode(GPin_AHHH, INPUT_PULLUP);
      pinMode(GPin_AHH, INPUT_PULLUP);
      pinMode(GPin_AH, INPUT_PULLUP);
      pinMode(GPin_AL, INPUT_PULLUP);
      Serial.println("debug level disabled");
    }
  }
  debugLevelSwitches_old = debugLevelSwitches;

  // **************************************************************************************************
  // **************************************************************************************************
  /* WebClient */

  seconds_since_startup = millis() / 1000;
  if ((clientIntervall > 0 && (seconds_since_startup - clientPreviousSs) >= clientIntervall)) // || (printOnChangeActive==1))
  {
    // sendPostToAskSensors(); // subscription cancelled

    if ((sendToClient == 1))
    {
      if (valueStable > 0)
      {
        sendPost(pumpStatus, pumpControl);
        // sendPost_V2();
      }
    }
    clientPreviousSs = seconds_since_startup;
  }
  server.handleClient();
  ElegantOTA.loop();

  // **************************************************************************************************
  // **************************************************************************************************
  /* evaluate water level */
  // **************************************************************************************************

  measureRuntimeStart(measureSensor);
  Current2Waterlevel();
  measureRuntimeEnd(measureSensor);

  SetAlarmState_from_relais();
  SetAlarmState_from_level();
  calculateWorstCaseAlarmState();

  prepareSendMail();

  // **************************************************************************************************
  // **************************************************************************************************
  /* Send Email reusing session   */
  // **************************************************************************************************

  if (executeSendMail)
  {
    SMTP_Message message;

    /* The attachment data item */
    SMTP_Attachment att[2];
    int attIndex = 0;
    // GZE  weitere Definitionen siehe ESP Mail Client/ examples / SMTP / Send_attachment_File

    message.sender.name = F("Pegel Zehentner");
    message.sender.email = AUTHOR_EMAIL;
    message.subject = subject;

    // somtimes we want to use web.de for test, because t-online has a huge delay
    if (useLiveMail == 1)
    {
      message.addRecipient(F("Schorsch"), RECIPIENT_EMAIL_LIVE);
    }
    else
    {
      message.addRecipient(F("Schorsch"), RECIPIENT_EMAIL);
    }

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

    // Send WhatsApp message
    if (executeSendMail)
    {
      Serial.println("Sending WhatsApp message");
      sendWhatsAppMessage(textMsg);
    };

  exit:

    heapInfo.collect();
    heapInfo.print();

    /*=END Send_Reuse_Session =====================================*/
  }

// **************************************************************************************************
// **************************************************************************************************
/*===========================================================
  simulate changing waterlevel
 */
#ifdef SIM_FADING_LEVEL
  // change the dutycylce for next time through the loop:
  pegel = pegel + fadeAmount;

  // reverse the direction of the fading at the ends of the fade:
  if (pegel <= Level_ALL / 100.0 - 0.05 || pegel >= Level_AHH / 100.0 + 0.05)
  {
    fadeAmount = -fadeAmount;
  }
#endif // SIM_FADING_LEVEL
// **************************************************************************************************

// **************************************************************************************************
#ifdef DEBUG_PRINT_RAW
  // **************************************************************************************************
  if (millis() - previousMillisCyclicPrint > WaitingTimeCyclicPrint)
  {

    /*=================================================================*/
    // read analog value via I2C for debug
    // not used in live system
    //===========================================
    const int loc_maxAdc_value = 0x7FFF;
    float voltage = 0.0;

    Serial.println("=================================");
    adc0 = GET_ANALOG;
    // adc0 = ads.readADC_Differential_0_1();
    Serial.print("Analog input pin 0: ");
    Serial.println(adc0);
    Serial.print("myValueFiltered: ");
    Serial.println(myValueFiltered);

    // voltage = ads.computeVolts(adc0);
    // Serial.print("Voltage: "); Serial.println(voltage);

    previousMillisCyclicPrint = millis();
  }
#endif // DEBUG_PRINT_RAW
       // **************************************************************************************************

  // generate a time tick for cyclic operations; e.g. debug print, but also blink LED
  if (millis() - previousMillisCyclicPrint > WaitingTimeCyclicPrint)

  {
    timeTickForCyclicPrint = 1;

    previousMillisCyclicPrint = millis();
  }
  else
  {
    timeTickForCyclicPrint = 0;
  }

// **************************************************************************************************
#ifdef DEBUG_PRINT_CYCLIC
  // **************************************************************************************************
  // Serial.print(formattedTime);
  // heapInfo.collect();
  // heapInfo.print();

  // Serial.print("millisNow : ");Serial.print(opTime_millisNow);Serial.print(" millisDiff : ");Serial.println(opTime_millisDiff);
  //  Serial.print(myValueFilteredAct);
  //  Serial.print(" - AlarmStateLevel: "); Serial.print(alarmStateLevel);
  //  Serial.print("  ");
  //  Serial.print("  pumpA: "); Serial.print(pumpA_op); Serial.print("  Op time 1 : "); Serial.print(pump1_operationTime);
  //  Serial.print("  pumpB: "); Serial.print(pumpB_op); Serial.print("  Op time 2 : "); Serial.print(pump2_operationTime);
  //  Serial.print(" linkPump : ");Serial.println(linkPump);
  if (timeTickForCyclicPrint == 1)
  {
    // Serial.println();
    // Serial.print("Control: "); Serial.print(manualPumpControl);
    // Serial.print("  Pump1 on:   "); Serial.print(manPump1Enabled);
    // Serial.print("  Pump2 on:   "); Serial.print(manPump2Enabled);
  }

#endif

  if (timeTickForCyclicPrint == 1)
  {
    if (digitalRead(builtin_led) == HIGH)
    {
      digitalWrite(builtin_led, LOW);
    }
    else
    {
      digitalWrite(builtin_led, HIGH);
    }
  }


  checkDailyTrigger(pumpStatus, pumpControl, triggerCheckPump);
  checkPump(pumpStatus, pumpControl, triggerCheckPump, dailyPumpRunTime);
  controlPump(pumpControl);
  measureOperatingTime(pumpStatus, pumpControl);
  selectPump(pumpStatus, pumpControl);
  manualPumpOperation(pumpControl);

  // input command via serial interface
  //**************************************************************************************

  millisNow = millis();

  if (millisNow - lastMillis >= 2000)
  {
    lastMillis = millisNow;
    doPrint = 1;

    // Serial.println(".");
    value++;
  }

  // serial input is not yet working reliably
#ifdef useSerialInput
  // if there's any serial available, read it:
  if (Serial.available() > 0)
  {

    receivedString = Serial.readString();

    Serial.println();
    Serial.println("rec: " + receivedString) + "---";
    Serial.println(";");

    Serial.println("len: " + (receivedString.length()));

    if (receivedString == "r")
    {
      Serial.println("executing readFile");
    }
  }
#endif // useSerialInput

  // **************************************************************************************************
  // SerialLink: Daten senden und empfangen
  // **************************************************************************************************
#if BOARDTYPE == ESP32
  // Eingehende Nachrichten verarbeiten
  if (serialLink.available()) {
    uint8_t msgType;
    uint8_t msgData[SL_MAX_DATA_LENGTH];
    uint8_t msgLength;
    
    if (serialLink.receiveMessage(msgType, msgData, msgLength)) {
      Serial.print(F("SerialLink RX: Type=0x"));
      Serial.print(msgType, HEX);
      Serial.print(F(", Length="));
      Serial.println(msgLength);
      
      // Nachricht verarbeiten
      switch (msgType) {
        case SL_MSG_PING:
          // Ping empfangen -> Pong senden
          serialLink.sendMessage(SL_MSG_PONG, nullptr, 0);
          Serial.println(F("  -> PING received, PONG sent"));
          break;
          
        case SL_MSG_TEXT:
          // Textnachricht empfangen
          msgData[msgLength] = '\0'; // Null-Terminierung
          Serial.print(F("  -> Text: "));
          Serial.println((char*)msgData);
          break;
          
        case SL_MSG_WATERLEVEL:
          // Wasserstand-Daten empfangen
          if (msgLength == sizeof(WaterLevelData)) {
            WaterLevelData data;
            memcpy(&data, msgData, sizeof(WaterLevelData));
            Serial.print(F("  -> WaterLevel: "));
            Serial.print(data.level_mm);
            Serial.print(F("mm, Alarm="));
            Serial.println(data.alarm_state);
          }
          break;
          
        default:
          Serial.println(F("  -> Unknown message type"));
          break;
      }
    }
  }
  
  // Eigene Daten periodisch senden
  if (seconds_since_startup - lastSerialLinkSend >= serialLinkInterval) {
    lastSerialLinkSend = seconds_since_startup;
    
    // Wasserstand-Daten zusammenstellen und senden
    WaterLevelData wlData;
    wlData.level_mm = myValueFilteredAct;
    wlData.adc_value = myAdcFilteredAct;
    wlData.alarm_state = alarmState;
    wlData.timestamp = seconds_since_startup;
    
    if (serialLink.sendWaterLevel(wlData)) {
      Serial.print(F("SerialLink TX: WaterLevel "));
      Serial.print(wlData.level_mm);
      Serial.println(F("mm sent"));
    }
    
    // Statistik ausgeben (alle 60 Sekunden)
    if (seconds_since_startup % 60 == 0) {
      Serial.print(F("SerialLink Stats: TX="));
      Serial.print(serialLink.getSentCount());
      Serial.print(F(", RX="));
      Serial.print(serialLink.getReceivedCount());
      Serial.print(F(", Errors="));
      Serial.println(serialLink.getErrorCount());
    }
  }
#endif

  // **************************************************************************************************
  // set a delay to avoid ESP is busy all the time
  //     allow the cpu to switch to other tasks

  delay(2);

  doPrint = 0;
  printOnChangeActive = 0;

  // **************************************************************************************************
  // measure max latency for loop run
  // **************************************************************************************************
  // get actual timestamp
  millisNowLoopRuntime = millis();
  // calculate actual loopRuntime
  actLoopRuntime = millisNowLoopRuntime - previousMillisLoopRuntime;

  // if actual loopRuntime is greater than max, this is the new max
  if (actLoopRuntime > maxLoopRuntime)
  {
    maxLoopRuntime = actLoopRuntime;
  }

  // remember actual timestamp as previous
  previousMillisLoopRuntime = millisNowLoopRuntime;

  measureRuntimeStart(measureLoopOthers);

  // **************************************************************************************************
  // **************************************************************************************************
} // end void loop()
  // **************************************************************************************************
  // **************************************************************************************************

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

//*******************************************************************************
// read values out of setup.ini
void getSetupIni(PumpStatus &pumpStatus)
//*******************************************************************************
{
  // check if file exists, if not, generate one with zero values
  if (!LittleFS.exists("/setup.ini"))
  {
    Serial.println("setup.ini does not exist / generate a new file");
    Serial.println("Calling putSetupIni");
    putSetupIni(pumpStatus);
  }
  // open file for reading
  File file = LittleFS.open("/setup.ini", "r");
  if (!file)
  {
    Serial.println("Failed to open setup.ini for reading");
    return;
  }
  else
  {
    Serial.println("setup.ini successfully opened for reading");
  }

  // read from file line by line
  // prepare loop
  // define locals
  char c;
  String FileContent = "";
  String fileLine = "";
  int isToken = 1;
  String tokenName = "";
  String tokenValue = "";

  while (file.available())
  {

    c = file.read();

    if (c == '\n')
    {
      // with every new line decode setting
      if (tokenName == "pump1_operationTime")
      {
        pumpStatus.pump1_operationTime = tokenValue.toInt();
      }
      if (tokenName == "pump2_operationTime")
      {
        pumpStatus.pump2_operationTime = tokenValue.toInt();
      }
      if (tokenName == "linkPump")
      {
        pumpStatus.linkPump = tokenValue.toInt();
      }

      // and prepare for next line
      isToken = 1;
      tokenName = "";
      tokenValue = "";
    }
    else if (c == '=')
    {
      // with every equ switch to value
      isToken = 0;
    }
    else if (c == ' ')
    {
      // space: do nothing
    }
    else
    {

      // get char and add to appropriate string
      if (isToken == 1)
      {
        tokenName += c;
      }
      else
      {
        tokenValue += c;
      }
    }
    // noValues ++;
  }
  file.close();
}
//*******************************************************************************
// write values to setup.ini
void putSetupIni(PumpStatus &pumpStatus)
//*******************************************************************************
{
  Serial.println("putSetupIni entered");
  String tempString = "";

  tempString += String("pump1_operationTime=");
  tempString += String(pumpStatus.pump1_operationTime);
  tempString += String(";\npump2_operationTime=");
  tempString += String(pumpStatus.pump2_operationTime);
  tempString += String(";\nlinkPump=");
  tempString += String(pumpStatus.linkPump);
  tempString += String(";\n");

  writeFile("/setup.ini", (tempString).c_str()); // Append data to the file
  Serial.println("data appended");
}
