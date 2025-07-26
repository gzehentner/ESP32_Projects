// define function to print debug messages
#ifndef DEBUGPRINT_H
#define DEBUGPRINT_H    

#include <Arduino.h>
#include <esp_system.h> // for esp_reset_reason_t
 //*********************************
// define log levels
const int LOG_LEVEL_DEBUG = 0;
const int LOG_LEVEL_INFO  = 1;
const int LOG_LEVEL_WARN  = 2;
const int LOG_LEVEL_ERROR = 3;
const int LOG_LEVEL_FATAL = 4;

const int globalDebugLevel = LOG_LEVEL_ERROR;
//*********************************

//*******************************************************************************
const char* espResetReasonToString(esp_reset_reason_t reason);

// void debugPrintln(String msg, int enableTelnet, int debugLevel);
// void debugPrint  (String msg, int enableTelnet, int debugLevel);
// void debugPrintln(int msg,    int enableTelnet, int debugLevel);
// void debugPrint  (int msg,    int enableTelnet, int debugLevel);

void debugPrintln(String msg);
void debugPrint  (String msg);
void debugPrintln(int msg   );
void debugPrint  (int msg   );
//*******************************************************************************
#endif