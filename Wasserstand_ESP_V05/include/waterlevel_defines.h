/*
 * 
 * Some basic defines for the whole project
 * 
 */

/*
 * Variables set in platformio.ini
 * - isLiveSystem
 * - MyUSE_ADC
 * 
 * 
 * *****************
 * \brief compiler switches
 * \param - BOARDTYPE: ESP8266 / ESP32
 * \param - 



*/

#ifndef WATERLEVEL_DEFINES_H
  #define WATERLEVEL_DEFINES_H

  #define VERSION "6.9" // the version of this sketch

  // setting for ADC select: MyUSE_ADC
  #define internADC 0
  #define ADS1115_ADC   1


  #ifdef ARDUINO_ARCH_ESP32          // if it is not a board with ESP32, then a
    #define BOARDTYPE ESP32
    // defined in platformio.ini 
    // #define MyUSE_ADC ADS1115_ADC
    //
  #else
    #define BOARDTYPE ESP8266          // Board with ESP8266 is used (internal ADC)
    // defined in platformio.ini 
    // #define MyUSE_ADC internADC
  #endif

#if MyUSE_ADC == ADS1115_ADC
  #if BOARDTYPE == ESP32
    #include <Wire.h>
    #include <Adafruit_Sensor.h>
    #include <Adafruit_ADS1X15.h>
  #else
    #include <Wire.h>
    #include <Adafruit_ADS1X15.h>
  #endif
#endif

  /* *******************************************************************
          the board settings / die Einstellungen der verschiedenen Boards
  ********************************************************************/
  #if isLiveSystem == 1
    #define TXT_BOARDID "164"                   // an ID for the board
    #define TXT_BOARDNAME "Wasserstand " // the name of the board
    #define CSS_MAINCOLOR "blue"                // don't get confused by the different webservers and use different colors
  #else
    #define TXT_BOARDID "Develop"           // show that something is configured as development version
    #define TXT_BOARDNAME "Wasserstand" // the name of the board
    #define CSS_MAINCOLOR "blue"                // don't get confused by the different webservers and use different colors
  #endif
  /*=================================================================*/
  /* Prepare WaterLevel Application */


  
  #if BOARDTYPE == ESP8266
    /* -- Pin-Def for ESP8266 -- */
    #define GPin_AHH   3 // weiß    // RX
    #define GPin_AH   13 // grau    // D7
    #define GPin_AL   12 // violett // D6
    // #define GPin_ALL  14 // blau   
    // #define GPout_GND 12 // schwarz
  #else
    /* -- Pin-Def for ESP32 -- */  // Board159
    #define GPin_AHH  2  // blau   rot
    #define GPin_AH  13  // grau   grün
    #define GPin_AL  12  // lila   gelb
    // #define GPin_ALL 16  // grün   orange (GPIO16 is not usable, because it is used for PSRAM)
  
  #endif

 
  // definitions for analog-digital conversion
  #if MyUSE_ADC == ADS1115_ADC
    #define ADC_BIT  15  // only 15 bit for single ended signals
    #define Ain_Level 0  // input = adc 

    #if BOARDTYPE == ESP32
      #define GET_ANALOG ads.readADC_Differential_0_1()
      #define I2C_SDA 14
      #define I2C_SCL 15
    #else
      #define GET_ANALOG ads.readADC_SingleEnded(0)
      #define I2C_SDA 4
      #define I2C_SCL 5
    #endif

  #else // use builtin ADC
    #if BOARDTYPE == ESP32
      Builtin ADC cannot be used with ESP32
    #endif

    #define GET_ANALOG analogRead(pin)
    #define ADC_BIT 10
    #define Ain_Level PIN_A0
  #endif

  #if BOARDTYPE == ESP32
        // pin of builtin led
    const int builtin_led = 4;
  #else
    #define builtin_led 2 
    #define BLUE_LED builtin_led
    #define PWM_OUT LED_BUILTIN
  #endif

  //**************************************************************
  //**************************************************************
  // global variables to simulate remote board
  //**************************************************************
  extern int remoteBoardId;
  extern int remoteLevelAct;
  extern int remoteDebugLevelSwitches;
  extern int remoteAHH;
  extern int remoteAH;
  extern int remoteAL;
  extern int remoteLastMessage;
  extern int remoteMessagesSucessfull;





  //**************************************************************
   //**************************************************************
   // Debug and Simulation settings used in development board
   //**************************************************************

  #if isLiveSystem == 1
  #else  
    //*********************************
    // enable printing in main loop
    // cyclic print with predefined time interval: WaitingCyclicPrint
    #define DEBUG_PRINT_CYCLIC
  
    //*********************************
    // print raw voltage values without calculating current values; used for debug ADC function
    // #define DEBUG_PRINT_RAW  

    //*********************************
    // use small values for loops to get a fast simulation of firmware    
    #define SIM_VALUES  

    //*********************************
    // generate a fading value for analog input to simulate functions without an external electronic
    // #define SIM_FADING_LEVEL  
    
    //*********************************
    // simulate level with poti 
    #define USE_POTI

  #endif

  // END: Debug and Simulation settings used in development board
  //**************************************************************

  //**************************************************************
  // Settings for data capture
  //**************************************************************
    const int  measureInterval = 100; // measurement interval in milliseconds

  // \warning GZE: values have to be checked and corrigated
  #ifdef SIM_VALUES
    const int  filterCntMax = 10;  // how many measurements are take for filter
                            // cycle time for myvalue: measureInterval * filterCntMax  -> 100 ms
    const unsigned long  longtermInterval = 10000;  // time between two saved values in longterm in ms
  #else
    const int  filterCntMax = 600;  //   / how many measurements are take for filter
                            // cycle time for myvalue: measureInterval * filterCntMax  -> here 60s
    const unsigned long  longtermInterval = 1000*60*60*6; // four times a day
  #endif

  //**************************************************************
  // settings for ring buffer
  // may be a too long ring buffer is the reason of sporadic crashes
  #if BOARDTYPE == ESP32
    //*****************
    // length of ring buffer
    #define iRingValueMax  1000 // new value every three minutes --> 720: buffer for one complete day (but then we get heap overflow)
    #define iLongtermRingValueMax 120 // 10// 120 // 370 // one value a day, buffer for one year (now we have four values a day so we have one month)
    //*****************
    // maximum lines to be printed and points in graph
    const int maxLines  = 1000;
    const int maxPoints = 1000;
  #else // BOARDTYPE == ESP8266
    //*****************
    // length of ring buffer
    // GZE \warning values have to be checked and adapted
    #define iRingValueMax         100 // new value every one minute --> 3h (in eval mode 180*10s)
    #define iLongtermRingValueMax 120 // four value a day, buffer for one month
    //*****************
    // maximum lines to be printed and points in graph
    const int maxLines  = 100;
    const int maxPoints = 100;
  #endif

  //**************************************************************
  /* -- Server / Client Settings -- */
  //**************************************************************
  #define CLIENT_INTERVALL_LIFE 60    // intervall to send data to a server in seconds. Set to 0 if you don't want to send data
  #define CLIENT_INTERVALL_DEV  10    // intervall to send data to a server in seconds. Set to 0 if you don't want to send data
  
  // if defined, send to my bplaced account
  #define sendToBplaced_sql

  // how many communication errors must occure before we reset ESP
  //**************************************************************
  #define ERR_CNT_COMMUNICATION 5
  
  //**************************************************************
  /* -- Alarm-Level -- */
  //**************************************************************
  // Pegel wurde verändert, hängt um 6cm höher
  #define Level_AHHH 195 // Oberkante Bodenplatte --> sofort zwei Pumpen einschalten
  #define Level_AHH  185 // Oberkante Schacht = 187cm // Oberkante Bodenplatte = (OK-Schacht + 8 cm) = 1,95cm
  #define Level_AH   170  // Warnschwelle
  #define Level_AL   155
  #define Level_ALL  140 // Unterkante KG Rohr
                         // Aktueller Niedrig-Stand Nov 2023 = 99 cm
  #define Level_HIST   2 // Hysterese für den Pegel

  #define Level_AHHH_Str "195"
  #define Level_AHH_Str  "185"
  #define Level_AH_Str   "170"
  #define Level_AL_Str   "155"
  #define Level_ALL_Str  "140" 

  // #define BUTTON1_PIN 19

  // value offset
  // adapt measured value to TS [cm]
  #define valOffset 0.
 
  /*=================================================================*/
  /* Parameter for CurrentLoop */
  /*=================================================================*/
  const byte sensorPin    = Ain_Level;  // ADC pin for the sensor
  const uint16_t resistor = 165;        // used shunt  resistor in Ohm
  const byte vref         = 32;         // VREF in Volt*10 (Uno 16MHz: 50, ProMini 8MHz: 3V3).

  #ifdef USE_POTI
    const int maxValue      = 250;        // measurement range: 000cm to 250cm --> maxValue=250
  #else
    const int maxValue      = 500;        // measurement range: 000cm to 500cm --> maxValue=500
  #endif

  /*=================================================================*/
  /* Parameter for pumpControl */
  /*=================================================================*/
  #if isLiveSystem == 1
    #define timeUnit_opTime 60000  // millis / 60000 = minutes
    #define opTimeToExchange 20    // unit: timeUnit_opTime (minutes)
    #define timeToSecondPump 5     // unit: timeUnit_opTime (minutes)
  #else
    #define timeUnit_opTime 1000   // (millis/1000) = seconds (for development)
    #define opTimeToExchange 20    // unit: timeUnit_opTime (seconds)
    #define timeToSecondPump 10    // unit: timeUnit_opTime (seconds)
  #endif


#endif
