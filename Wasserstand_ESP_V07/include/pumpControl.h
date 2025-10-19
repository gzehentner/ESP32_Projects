/***********************************************************************************
 *
 * pumpControl.h
 *
 */

//*******************************************************************************

#ifndef PUMPCONTROL_H
#define PUMPCONTROL_H

#include <Arduino.h>
// #include <timeserver.h>

// variables and constants

struct PumpStatus
{
    int pump1_operationTime; // operating time for pump A in minutes
    int pump2_operationTime; // operating time for pump B in minutes
    int linkPump;            // logical connection
                             // 0:  1 <-> A  // 2 <-> B
                             // 1:  1 <-> B  // 2 <-> A
};

struct PumpControl
{
    int pump1_op; // indicator, that pump1 is running
    int pump2_op; // indicator, that pump2 is running

    int pumpA_op; // indicator, that pumpA is running
    int pumpB_op; // indicator, that pumpB is running
};

//*******************************************************************************

void measureOperatingTime(PumpStatus &pumpStatus, PumpControl &pumpControl);
void controlPump(PumpControl &pumpControl);
void selectPump(PumpStatus &pumpStatus, PumpControl &pumpControl);

#endif