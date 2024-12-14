/********************************************************************
         Declaration of globals - Variables and constants
 ********************************************************************/

#ifndef WATERLEVEL_H
#define WATERLEVEL_H

#include <Arduino.h>

extern int val_AH;
extern int val_AL;
extern int val_AHH;
extern int val_AHHH;
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

extern int alarmStateRelais   ; // actual state of alarm derived from relais
extern int alarmStateRelaisOld; // previous value of alarmStateRelais
extern int alarmStateLevel    ; // actual state of alarm derived from measured water level
extern int alarmStateLevelOld ; // previous value of alarmStateLevel
extern int alarmState   ;       // worst case value of both alarmState
extern int alarmStateOld;       // previous value of alarmState

extern bool executeSendMail;

extern int myValueFilteredAct; // actual result for display in web page
extern int myAdcFilteredAct;

extern int filterCnt;   
extern unsigned long previousMillis;             // used to determine intervall of ADC measurement

extern unsigned long millisNow;   

// variables for shortterm ringbuffer
extern unsigned long ringTime[iRingValueMax+1];    // ring buffer for display last n values
extern int    ringValue[iRingValueMax+1];
extern int    ringADC[iRingValueMax+1];

extern int wrRingPtr;           // index variable for write buffer
extern int rdRingPtr;            // position to read out of buffer


//*******************************************************************************
// write values to setup.ini
void putSetupIni();
//*******************************************************************************
// read values out of setup.ini
void getSetupIni();
//*******************************************************************************



// definitions for analog-digital conversion (using external ADS1115)
  #if MyUSE_ADC == ADS1115_ADC
    #if BOARDTYPE == ESP32
      // TwoWire I2CSensors = TwoWire(0);
      extern Adafruit_ADS1115 ads;
      // int16_t adc0;
    #else
      extern Adafruit_ADS1115 ads;
      //Wire.begin(4,5);
    #endif
  #endif
#endif