/********************************************************************
         Declaration of globals - Variables and constants
 ********************************************************************/

#ifndef WATERLEVEL_H
#define WATERLEVEL_H

#include <Arduino.h>

extern int val_AH;
extern int val_AL;
extern int val_AHH;
extern int val_ALL;

extern unsigned long seconds_since_startup;      // current second since startup
extern unsigned long seconds_since_startup_last; // value of ss out of the last loop run
extern const uint16_t ajaxIntervall;             // intervall for AJAX or fetch API call of website in seconds
extern uint32_t clientPreviousSs;                // - clientIntervall;
                                                 // last second when data was sent to server

extern const uint16_t clientIntervall; // intervall to send data to a server in seconds. Set to 0 if you don't want to send data
extern const char *sendHttpTo;         // the module will send information to that server/resource. Use an URI or an IP address

// extern int inByte;
// extern int incomingByte; // for incoming serial data

extern int alarmState;    // shows the actual water level
extern int alarmStateOld; // previous value of alarmState
extern bool executeSendMail;

extern int myValueFiltered;   // result of the filtering
extern int myValueFilteredAct; // actual result for display in web page
extern int filterCnt;   
extern unsigned long previousMillis;             // used to determine intervall of ADC measurement
extern unsigned long longtermPreviousMillis;     // used to determine intervall since last value saved

extern unsigned long millisNow;   

extern unsigned long ringTime[iRingValueMax+1];    // ring buffer for display last 100 values
extern int    ringValue[iRingValueMax+1];
extern int    ringADC[iRingValueMax+1];

extern int wrRingPtr;           // index variable for write buffer
extern int rdRingPtr;            // position to read out of buffer

// longterm 
extern unsigned long ringLongtermTime [iLongtermRingValueMax +1];
extern int    ringLongtermValue[iLongtermRingValueMax +1];     // ring buffer for display last 50 values

extern int    wrLongtermRingPtr;                  // ring buffer write pointer 
extern int    rdLongtermRingPtr;                  // ring buffer read pointer 

// GZE
extern float pegel; // waterlevel in m


// definitions for analog-digital conversion
  #ifdef USE_ADS1115
     extern TwoWire I2CSensors;
     extern Adafruit_ADS1115 ads;
   #endif

#endif