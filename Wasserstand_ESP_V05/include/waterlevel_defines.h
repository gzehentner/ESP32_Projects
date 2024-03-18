/*

Some basic defines for the whole project

*/

#ifndef WATERLEVEL_DEFINES_H
#define WATERLEVEL_DEFINES_H

#define VERSION "5.1" // the version of this sketch

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADS1X15.h>



/* *******************************************************************
         the board settings / die Einstellungen der verschiedenen Boards
 ********************************************************************/

#define BOARDTYPE  ESP32                  // Board with ESP32 is used (external ADC)
// #define BOARDTYPE  ESP8266                // Board with ESP8266 is used (internal ADC)
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
#else
   #define GET_ANALOG ads.readADC_SingleEnded
   #define ADC_BIT  15  // only 15 bit for single ended signals
   #define Ain_Level 0  // input = adc0

   #define I2C_SDA 14
   #define I2C_SCL 15

#endif


// pin of builtin led
const int builtin_led = 4;

#define PWM_OUT builtin_led // PWM-Output is set to BUILTIN_LED




/* -- Alarm-Level -- */
// Pegel wurde verändert, hängt um 6cm höher
// --> Werte korrigieren
#define Level_AHH 185 // Oberkante Schacht = 191cm
#define Level_AH  170  // Warnschwelle
#define Level_AL  155
#define Level_ALL 140 // Unterkante KG Rohr
                      // Aktueller Niedrig-Stand Nov 2023 = 99 cm

// #define DEBUG_PRINT_RAW  // read ADS and print value for debug
// #define SIM_FADING_LEVEL // simulate a changing waterlevel using analog-output

#define SIM_VALUES  // use small values for loops to get a fast simulation of firmware
#endif