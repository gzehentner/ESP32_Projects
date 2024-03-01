/*

Some basic defines for the whole project

*/

#ifndef WATERLEVEL_DEFINES_H
#define WATERLEVEL_DEFINES_H

#define VERSION "5.0" // the version of this sketch

/* *******************************************************************
         the board settings / die Einstellungen der verschiedenen Boards
 ********************************************************************/

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
// #define GPout_GND 12

#define Ain_Level 12 

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

// #define Level_AHH 190 // Oberkante Schacht = 197cm
// #define Level_AH 185  // Zwischenstand
// #define Level_AL 180
// #define Level_ALL 145 // Unterkante KG Rohr
//                       // Aktueller Niedrig-Stand Nov 2023 = 105cm

// #define DEBUG_PRINT_RAW
#define SIM_VALUES  // use small values for loops to get a fast simulation of firmware

#endif