/*
=============================================
Spielwiese:
Anwendung Wasserstand zur Version 6.9
Bei der Kopie aus dem Live-Programm Wasserstand_V5 wurden eine Datei test.php und eine Funktion mit hinzugefügt, mit dem
man testen kann, wie man eine HTML-Form zum Eingeben von Daten nutzen kann
Umbenennung (Projekt und Main-File) in Wasserstand_Dev
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

  #include <ESP_Mail_Client.h>
  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>

// use time.h from Arduino.h 
#include "time.h"                   // for time() ctime()

#include <ArduinoOTA.h>   // OTA Upload via ArduinoIDE


  #include "soc/soc.h"            // disable brownout detector
  #include "soc/rtc_cntl_reg.h"   // disable brownout detector

  WebServer server(80);

  #include <server.h>
  #include <timeserver.h>
  #include <NoiascaCurrentLoop.h>   // library for analog measurement
  #include <EvaluateSensor.h>


  #include <ProjClient.h>

  #include <pumpControl.h>
  
  #include <MyLittleFSLib.h>

#else
  #include <Arduino.h>

  #include <waterlevel.h>

  #include <ESP_Mail_Client.h>
  #include <ESP8266mDNS.h>  // Bonjour/multicast DNS, finds the device on network by name
  #include <ArduinoOTA.h>        // OTA Upload via ArduinoIDE

  #include <server.h>
  #include <timeserver.h>
  #include <NoiascaCurrentLoop.h>   // library for analog measurement
  #include <EvaluateSensor.h>

  #include <ProjClient.h>
  #include <LittleFS.h>

  #include <pumpControl.h>
  
  #include <MyLittleFSLib.h>
#endif

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

unsigned long seconds_since_startup = 0;      // current second since startup

unsigned long previousMillis = 0;             // used to determine intervall of ADC measurement
unsigned long longtermPreviousMillis = 0;     //
unsigned long millisNow      = 0;  

const uint16_t ajaxIntervall = 5;             // intervall for AJAX or fetch API call of website in seconds
uint32_t clientPreviousSs = 0;                // - clientIntervall;  // last second when data was sent to server

#if isLiveSystem == 1  /* dont send */
  const uint16_t clientIntervall = CLIENT_INTERVALL_LIFE;         // intervall to send data to a server in seconds. Set to 0 if you don't want to send data
                                               // live systen shall not send anything for now
#else
  const uint16_t clientIntervall = CLIENT_INTERVALL_DEV;       // intervall to send data to a server in seconds. Set to 0 if you don't want to send data
#endif

// select http url to send to: 
//  - send to Bplaced or an internal device
//  - select if it is a live system or development
#ifdef sendToBplaced_sql
  #if isLiveSystem == 1
    const char *sendHttpTo = "http://zehentner.bplaced.net/Wasserstand/live/data_sql.php"; // the module will send information to that server/resource. Use an URI or an IP address
  #else
    const char *sendHttpTo = "http://zehentner.bplaced.net/Wasserstand/dev/data_sql.php"; // the module will send information to that server/resource. Use an URI or an IP address
  #endif
#else
  #if isLiveSystem == 1
    const char *sendHttpTo = "http://192.168.178.155/d.php"; // the module will send information to that server/resource. Use an URI or an IP address
  #else 
    const char *sendHttpTo = "http://192.168.178.164/d.php"; // the module will send information to that server/resource. Use an URI or an IP address
  #endif
#endif




int alarmStateRelais    = 0; // actual state of alarm derived from relais
int alarmStateRelaisOld = 0; // previous value of alarmStateRelais
int alarmStateLevel     = 0; // actual state of alarm derived from measured water level
int alarmStateLevelOld  = 0; // previous value of alarmStateLevel
int alarmState    = 0;       // worst case value of both alarmState
                             //   5 : pump running
                             //   4 : severe warning
                             //   3 : normal high
                             //   2 : normal 
                             //   1 : normal low
int alarmStateOld = 0;       // previous value of alarmState
bool executeSendMail = false;

int debugLevelSwitches_old = 0;
String receivedString="";

char input[64];
bool inputReadingCompleted = false;

long lastMillis=0;


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
#if isLiveSystem == 1
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

#ifndef STASSID                // either use an external .h file containing STASSID and STAPSK or ...
#define STASSID "Zehentner"    // ... modify these line to your SSID
#define STAPSK "ElisabethScho" // ... and set your WIFI password
#endif

const char *ssid = STASSID;
const char *password = STAPSK;


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


 void flushInput(){
  for (int i = 0; i < 64; i++){
    input[i] = {};
  }
 }
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
  /* =================  setup serial     */
  Serial.begin(115200);
  
  // wait for serial to come online
  while(!Serial); 
  Serial.println("Serial is ready to accept input");
  //------------------------

  Serial.println(F("\n" TXT_BOARDNAME "\nVersion: " VERSION " Board " TXT_BOARDID " "));
  Serial.print(__DATE__);
  Serial.print(F(" "));
  Serial.println(__TIME__);

   /*=================  Connect to WIFI */
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

  pinMode(GPin_AHHH, INPUT_PULLUP);
  pinMode(GPin_AHH,  INPUT_PULLUP);
  pinMode(GPin_AH,   INPUT_PULLUP);
  pinMode(GPin_AL,   INPUT_PULLUP);

  digitalWrite(GPout_pump1,0);
  digitalWrite(GPout_pump2,0);
  pinMode(GPout_pump1, OUTPUT);
  pinMode(GPout_pump2, OUTPUT);


 
  /* ----End Setup WaterLevel ------------------------------------------ */
  /*=================================================================*/

  /*=================================================================*/
  /*=================================================================*/
  /* Setup WebServer and start*/
  /*=================================================================*/
  
  // define the pages and other content for the webserver
  server.on("/", handlePage);      // send root page
  server.on("/0.htm", handlePage); // a request can reuse another handler
  // problems with too big data --> still disabled
  //server.on("/graph_poc.htm", handleGraph_POC); // display a chart with print on change values based on google graph
  server.on("/graph.htm", handleGraph); // display a chart with shortterm values
  server.on("/longterm_graph.htm", handleLongtermGraph);
  server.on("/filtered.htm",handleListFiltered);

  server.on("/f.css", handleCss); // a stylesheet
  server.on("/j.js", handleJs);   // javscript based on fetch API to update the page
  //server.on("/jslider.js", handleSliderJs);   // javscript display of slider value
  // server.on("/j.js",  handleAjax);             // a javascript to handle AJAX/JSON update of the page  https://werner.rothschopf.net/201809_arduino_esp8266_server_client_2_ajax.htm
  server.on("/json", handleJson);    // send data in JSON format
  server.on("/c.php", handleCommand);            // process commands
                                     //  server.on("/favicon.ico", handle204);          // process commands
  server.onNotFound(handleNotFound); // show a typical HTTP Error 404 page
  //server.on("/slider.htm",handleSlider);

  // the next two handlers are necessary to receive and show data from another module
  server.on("/d.php", handleData);               // receives data from another module
  server.on("/r.htm", handlePageR);              // show data as received from the remote module
  server.on("/test.htm",handleHtmlFile);         // test procedure to handle reading html code from file
  server.on("/testR.htm",handleRawText);         // test procedure to handle reading html code as raw string
//  server.on("/set_time_steps.htm", HTTP_POST, handleSetTimeSteps);


  // following settings are coming from AdvancedWebServer
  // server.on("/test.svg", drawGraph);
  // server.on("/inline", []() {
  //   server.send(200, "text/plain", "this works as well");
  // });

  server.begin(); // start the webserver
  Serial.println(F("HTTP server started"));
  /*=====================   end fo prepare webserver  ===============*/
  
  /*=================================================================*/
  /* IDE OTA */
  ArduinoOTA.setHostname(myhostname); // give a name to your ESP for the Arduino IDE
  ArduinoOTA.begin();                 // OTA Upload via ArduinoIDE https://arduino-esp8266.readthedocs.io/en/latest/ota_updates/readme.html



  
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
      Wire.begin(I2C_SDA,I2C_SCL);
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
      //while  (1) // endless loop is not good, because ESP is not updatable by OTA
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

  //==========================================
  // prepare logfile

  // Serial.println("Formatting LittleFS filesystem");
  // LittleFS.format();

  Serial.println("Mount LittleFS");
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    return;
  }
  
  // this var is set in platformio.ini (this doesnot work??)
  #define deleteLogfile 0
  #if deleteLogfile == 1
    // deleting logfile
    Serial.println("deleting logfile");
    deleteFile("/level.log");
  #else
    Serial.println("NOT!!! deleting logfile");
  #endif

  #define deleteSetupFile 0
  #if deleteSetupFile == 1
    // deleting setupFile
    Serial.println("deleting setupFile");
    deleteFile("/setup.ini");
  #else
    Serial.println("NOT!!! deleting setup.ini");
  #endif

  getSetupIni();
  //readFile("/setup.ini");
  Serial.println();
  Serial.print("pump1_operationTime : ");Serial.println(pump1_operationTime);
  Serial.print("pump2_operationTime : ");Serial.println(pump2_operationTime);
  Serial.print("linkPump            : ");Serial.println(linkPump);

  #define deleteErrLog 0
  #if deleteErrLog == 1
    // deleting setupFile
    Serial.println("deleting error.log");
    deleteFile("/error.log");
  #else
    Serial.println("NOT!!! deleting error.log");
  #endif

  readFile("/error.log");
 

  //listDir("/");  
}
  /*==================================================================*/
  

/*****************************************************************************************************************
 *****************************************************************************************************************
         M A I N L O O P
 *****************************************************************************************************************
 *****************************************************************************************************************/
 

void loop(void) {

  // **************************************************************************************************
  // ===============================================================================
  // following actions are triggered every 500 ms
  if (millis() - previousMillis_halfSecondAction > halfSecond)
  {

    /*=================================================================*/
    /*  code for getting time from NTP       */
    getEpochTime(epochTime);
    formatDateAndTime(formattedTime, currentDate,epochTime);
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
  if (debugLevelSwitches != debugLevelSwitches_old) {
    if (debugLevelSwitches == 1) {
      // set default values to last actual values, when debug is switched on 
      Serial.print("val_AHHH: "); Serial.println(val_AHHH);
      Serial.print("val_AHH:  "); Serial.println(val_AHH);
      Serial.print("val_AH:   "); Serial.println(val_AH);
      Serial.print("val_AL:   "); Serial.println(val_AL);
      // simVal_AHHH = val_AHHH;
      simVal_AHH =  val_AHH;
      simVal_AH  =  val_AH;
      simVal_AL  =  val_AL;
      pinMode(GPin_AHHH, OUTPUT);
      pinMode(GPin_AHH, OUTPUT);
      pinMode(GPin_AH,  OUTPUT);
      pinMode(GPin_AL,  OUTPUT);
      
      // digitalWrite(GPin_AHHH, simVal_AHHH);
      digitalWrite(GPin_AHH , simVal_AHH);
      digitalWrite(GPin_AH  , simVal_AH);
      digitalWrite(GPin_AL  , simVal_AL);
      Serial.println("debug level enabled");
    } else {
      pinMode(GPin_AHHH, INPUT_PULLUP);
      pinMode(GPin_AHH , INPUT_PULLUP);
      pinMode(GPin_AH  , INPUT_PULLUP);
      pinMode(GPin_AL  , INPUT_PULLUP);
      Serial.println("debug level disabled");
    }
  }
  debugLevelSwitches_old = debugLevelSwitches;

  // **************************************************************************************************
  // **************************************************************************************************
  /* WebClient */

  seconds_since_startup = millis() / 1000;
  if ((clientIntervall > 0 && (seconds_since_startup - clientPreviousSs) >= clientIntervall) ) // || (printOnChangeActive==1))
  {
    // sendPostToAskSensors(); // subscription cancelled

    // send to client in live system on any case
    // send to client in develop system only on demand
    if ((sendToClient==1) || (isLiveSystem==1)) {
      if (valueStable>0) {
        sendPost();
        //sendPost_V2();
      }
    }
    clientPreviousSs = seconds_since_startup;
  }
  server.handleClient();

  // **************************************************************************************************
  // **************************************************************************************************

  /* Over the Air UPdate */
  //ArduinoOTA.handle(); // OTA Upload via ArduinoIDE

  // **************************************************************************************************
  // **************************************************************************************************
  /* evaluate water level */
  // **************************************************************************************************

  Current2Waterlevel();
 
  SetAlarmState_from_relais();
  SetAlarmState_from_level();
  calculateWorstCaseAlarmState();

  prepareSendMail();

  // **************************************************************************************************
  // **************************************************************************************************
  /* Send Email reusing session   */
  // **************************************************************************************************

  if (false) // GZE debug
  //if (executeSendMail)
  {
     Serial.println("Sending email disabled for test");
     executeSendMail = false;

    SMTP_Message message;
    
    /* The attachment data item */
    SMTP_Attachment att[2];
    int attIndex = 0;
    // GZE  weitere Definitionen siehe ESP Mail Client/ examples / SMTP / Send_attachment_File

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
  
  // **************************************************************************************************
  // **************************************************************************************************
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
  // **************************************************************************************************


  // **************************************************************************************************
  #ifdef DEBUG_PRINT_RAW
  // **************************************************************************************************
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
  // **************************************************************************************************


  
  // **************************************************************************************************
  #ifdef DEBUG_PRINT_CYCLIC
  // **************************************************************************************************
    if (millis() - previousMillisCyclicPrint > WaitingTimeCyclicPrint)
    {
      // Serial.print(formattedTime);
      // heapInfo.collect();
      // heapInfo.print();
      
      //Serial.print("millisNow : ");Serial.print(opTime_millisNow);Serial.print(" millisDiff : ");Serial.println(opTime_millisDiff);
      // Serial.print(myValueFilteredAct);
      // Serial.print(" - AlarmStateLevel: "); Serial.print(alarmStateLevel);
      // Serial.print("  ");
      // Serial.print("  pumpA: "); Serial.print(pumpA_op); Serial.print("  Op time 1 : "); Serial.print(pump1_operationTime);
      // Serial.print("  pumpB: "); Serial.print(pumpB_op); Serial.print("  Op time 2 : "); Serial.print(pump2_operationTime);
      // Serial.print(" linkPump : ");Serial.println(linkPump);
      
      

      previousMillisCyclicPrint = millis();
    }
  #endif


  controlPump();
  selectPump();
  measureOperatingTime();
  

  // input command via serial interface
  //**************************************************************************************


  millisNow = millis();

  if (millisNow - lastMillis >= 2000) {
    lastMillis = millisNow;
    doPrint = 1;

    //Serial.println(".");
    value ++;
  }

  // serial input is not yet working reliably
#ifdef useSerialInput
// if there's any serial available, read it:
  if (Serial.available() > 0) {

    receivedString= Serial.readString();

    Serial.println();
    Serial.println("rec: " + receivedString) + "---";
    Serial.println(";");

    Serial.println("len: " + (receivedString.length()));

    if (receivedString == "r") {
      Serial.println("executing readFile");
      
    }

    // 'R': read complete file at once 
    // if (c == 'R') {
    //   Serial.print("2");

    //   Serial.println("executing readFile");
    //   readFile("/level.log");
      
    // }
  

    // // "r": read file line by line
    // if (c == 'r'){
    //   Serial.print("3");
    //   // open file for reading and check if it exists
    //   File file = LittleFS.open("/level.log", "r");
    //   if (!file) {
    //     Serial.println("Failed to open file for reading");
    //     return;
    //   }

    //   while (file.available()) { 
    //     String fileData = "";
    
    //     fileData  = file.read();
    //     Serial.print ("fileData : "); Serial.println(fileData);
    
    //   }
    
   }
   #endif //useSerialInput

   
  
  // **************************************************************************************************
  // set a delay to avoid ESP is busy all the time
  //     allow the cpu to switch to other tasks
  
  delay(2);

  doPrint = 0;
  printOnChangeActive = 0;
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
void getSetupIni()
//*******************************************************************************
{
  // open file for reading and check if it exists
  File file = LittleFS.open("/setup.ini", "r");
  if (!file) {
    Serial.println("Failed to open setup.ini for reading");
    return;
  }

  // read from file line by line
  // prepare loop
  // define locals
  char c;
  String FileContent="";
  String fileLine="";
  int isToken  = 1;
  String tokenName  ="";
  String tokenValue ="";
    
  while (file.available()) { 

    c = file.read();

    if  (c=='\n'){
      // with every new line decode setting
        if (tokenName == "pump1_operationTime") {
            pump1_operationTime = tokenValue.toInt();
        }
        if (tokenName == "pump2_operationTime") {
            pump2_operationTime = tokenValue.toInt();
        }
        if (tokenName == "linkPump") {
            linkPump = tokenValue.toInt();
        }

        // and prepare for next line
        isToken=1;
        tokenName="";
        tokenValue="";

        } else if (c=='='){
        // with every equ switch to value
        isToken=0;
        } else if (c==' '){
        // space: do nothing
        } else {

        // get char and add to appropriate string
        if (isToken==1){
            tokenName+= c;
        } else {
            tokenValue+= c;
        }
    }
    //noValues ++;
  }
  file.close();

}
//*******************************************************************************
// write values to setup.ini
void putSetupIni()
//*******************************************************************************
{
    String tempString="";

    tempString += "pump1_operationTime="    + String(pump1_operationTime);
    tempString += ";\npump2_operationTime=" + String(pump2_operationTime);
    tempString += ";\nlinkPump="            + String(linkPump);
    tempString += ";\n";

    writeFile("/setup.ini", (tempString).c_str()); // Append data to the file
  
        
}