/***********************************************************************************
 * 
 * pumpControl.h
 * 
 */

//*******************************************************************************

#ifndef PUMPCONTROL_H
#define PUMPCONTROL_H

#include <Arduino.h>
//#include <timeserver.h>

// variables and constants

extern unsigned long pumpA_operationTime;  // operating time for pump A in minutes
extern unsigned long pumpB_operationTime;  // operating time for pump B in minutes

extern int pumpA_op;       // indicator, that pumpA is running
extern int pumpB_op;       // indicator, that pumpB is running

extern unsigned long opTime_millisNow;
extern unsigned long opTime_millisDiff;
extern unsigned long opTime_previousMillis;


//*******************************************************************************

void measureOperatingTime();
void controlPump();

#endif