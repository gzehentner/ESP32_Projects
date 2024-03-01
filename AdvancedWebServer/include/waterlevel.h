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

extern String graphXValues;      // values for graph
extern String graphYValues;
extern String graphXValuesTmp;      
extern String graphYValuesTmp;
extern String graphYlevelWarn;
extern String graphYlevelErro;
extern String graphYlevelWarnTmp;
extern String graphYlevelErroTmp;

extern const int iRingValueMax;

#ifdef SIM_VALUES
extern String ringTime[10+1];    // ring buffer for display last 100 values
extern int    ringValue[10+1];
extern int    ringADC[10+1];
#else
extern String ringTime[50+1];    // ring buffer for display last 100 values
extern int    ringValue[50+1];
extern int    ringADC[50+1];
#endif
extern int wrRingPtr;           // index variable for write buffer
extern int rdRingPtr;            // position to read out of buffer


#endif