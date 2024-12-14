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

extern unsigned long pump1_operationTime;  // operating time for pump A in minutes
extern unsigned long pump2_operationTime;  // operating time for pump B in minutes

extern int pumpA_op;       // indicator, that pumpA is running
extern int pumpB_op;       // indicator, that pumpB is running

extern int pump1_op;       // indicator, that pump1 is running
extern int pump2_op;       // indicator, that pump2 is running

extern int linkPump;       // logical connection
                           // 0:  1 <-> A  // 2 <-> B
                           // 1:  1 <-> B  // 2 <-> A

extern unsigned long opTime_millisNow;
extern unsigned long opTime_millisDiff;
extern unsigned long opTime_previousMillis;

extern unsigned long start2ndPump_millisNow;
extern unsigned long start2ndPumpNow;
extern unsigned long startPumpA_millis;

//*******************************************************************************

void measureOperatingTime();
void controlPump();
void selectPump();

#endif