/*

Some basic defines for the whole project

*/

#ifndef WATERLEVEL_DEFINES_H
#define WATERLEVEL_DEFINES_H

#define VERSION "5.0" // the version of this sketch

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
#define GPin_AHH 16 // rot
#define GPin_AH   2  // orange
#define GPin_AL  14  // gelb
#define GPin_ALL 15  // grün

#if BOARDTYPE == ESP8266
  #define GPout_GND 12
#endif

// definitions for analog-digital conversion
#if BOARDTYPE == ESP8266
   #define GET_ANALOG analogRead
   #define ADC_BIT 10
   #define Ain_Level 12 
#else
   #define GET_ANALOG get_spi_value
   #define ADC_BIT 14
   #define Ain_Level 12 
#endif


#define PWM_OUT 13 // PWM-Output to IO1

const int led = 4;


/* -- Alarm-Level -- */
// Pegel wurde verändert, hängt um 6cm höher
// --> Werte korrigieren
#define Level_AHH 185 // Oberkante Schacht = 191cm
#define Level_AH  170  // Warnschwelle
#define Level_AL  155
#define Level_ALL 140 // Unterkante KG Rohr
                      // Aktueller Niedrig-Stand Nov 2023 = 99 cm

// #define DEBUG_PRINT_RAW
#define SIM_VALUES  // use small values for loops to get a fast simulation of firmware

#endif