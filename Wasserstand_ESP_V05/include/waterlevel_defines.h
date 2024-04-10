/*

Some basic defines for the whole project

*/

#ifndef WATERLEVEL_DEFINES_H
  #define WATERLEVEL_DEFINES_H

  #define VERSION "5.1" // the version of this sketch

  #define BOARDTYPE ESP32                    // Board with ESP32 is used (external ADC)
  // #define BOARDTYPE ESP8266                // Board with ESP8266 is used (internal ADC)

  #if BOARDTYPE == ESP32
    #include <Wire.h>
    #include <Adafruit_Sensor.h>
    #include <Adafruit_ADS1X15.h>
  #endif



  /* *******************************************************************
          the board settings / die Einstellungen der verschiedenen Boards
  ********************************************************************/

  #define TXT_BOARDID "164"                   // an ID for the board
  #define TXT_BOARDNAME "Wasserstand-Messung" // the name of the board
  #define CSS_MAINCOLOR "blue"                // don't get confused by the different webservers and use different colors

  /*=================================================================*/
  /* Prepare WaterLevel Application */

  /* -- Pin-Def -- */
  #define GPin_AHH 16  // rot
  #define GPin_AH   2  // orange
  #define GPin_AL  12  // gelb
  #define GPin_ALL 13  // grün

  #if BOARDTYPE == ESP8266
    #define GPout_GND 12
  #endif

  // definitions for analog-digital conversion
  #if BOARDTYPE == ESP8266
    #define GET_ANALOG analogRead
    #define ADC_BIT 10
    #define Ain_Level 12 
    #define PWM_OUT 1 // PWM-Output to IO1

    #define builtin_led 2
    #define BLUE_LED builtin_led
    
  #else // BOARDTYPE == ESP32
    #define GET_ANALOG ads.readADC_SingleEnded
    #define ADC_BIT  15  // only 15 bit for single ended signals
    #define Ain_Level 0  // input = adc0

    #define I2C_SDA 14
    #define I2C_SCL 15

    // pin of builtin led
    const int builtin_led = 4;

    #define PWM_OUT builtin_led // PWM-Output is set to BUILTIN_LED
  #endif

  /* -- Alarm-Level -- */
  // Pegel wurde verändert, hängt um 6cm höher
  // --> Werte korrigieren
  #define Level_AHH 185 // Oberkante Schacht = 191cm
  #define Level_AH  170  // Warnschwelle
  #define Level_AL  155
  #define Level_ALL 140 // Unterkante KG Rohr
                        // Aktueller Niedrig-Stand Nov 2023 = 99 cm

  // #define Level_AHH 190 // Oberkante Schacht = 197cm
  // #define Level_AH 185  // Zwischenstand
  // #define Level_AL 180
  // #define Level_ALL 145 // Unterkante KG Rohr
  //                       // Aktueler Niedrig-Stand Nov 2023 = 105cm



  #define DEBUG_PRINT_RAW
  #define SIM_VALUES  // use small values for loops to get a fast simulation of firmware
  #define SIM_FADING_LEVEL

  // length of ring buffer
  #define iRingValueMax  100 // 50 // new value every three minutes --> 720: buffer for one complete day (but then we get heap overflow)
  #define iLongtermRingValueMax 120 // 10// 120 // 370 // one value a day, buffer for one year (now we have four values a day so we have one month)
  // maximum lines to be printed and points in graph
  const int maxLines = 10;
  const int maxPoints = 10;

  #ifdef SIM_VALUES
    const int  filterCntMax = 10;        // time for myvalue: measureInterval * filterCntMax  
    const unsigned long  longtermInterval = 10000;  // time between two saved values in ms
  #else
    const int  filterCntMax = 1800; // time for myvalue: measureInterval * filterCntMax  1200 : every three minutes
    const unsigned long  longtermInterval = 1000*60*60*6; // four time a day
  #endif

  /* -- Alarm-Level -- */
  // Pegel wurde verändert, hängt um 6cm höher
  // --> Werte korrigieren
  #define Level_AHH 185 // Oberkante Schacht = 191cm
  #define Level_AH  170  // Warnschwelle
  #define Level_AL  155
  #define Level_ALL 140 // Unterkante KG Rohr
                        // Aktueller Niedrig-Stand Nov 2023 = 99 cm

  // #define DEBUG_PRINT_RAW  // read ADS and print value for debug
  // #define DEBUG_PRINT_HEAP
  #define SIM_FADING_LEVEL // simulate a changing waterlevel using analog-output

  #define SIM_VALUES  // use small values for loops to get a fast simulation of firmware
#endif