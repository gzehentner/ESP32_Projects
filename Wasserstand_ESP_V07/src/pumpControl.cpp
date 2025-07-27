/***********************************************************************************
 *
 * pumpControl.cpp
 *
 * \brief control the two pumps
 *  - control in case of high waterlevel
 *  - change master-slave-pump to get approximately same operating time
 *  - control regulary testruns
 */

//*******************************************************************************
#include <Arduino.h>
#include <waterlevel_defines.h>
#include <waterlevel.h>
#include <timeserver.h>
#include <pumpControl.h>

// variables and constants

// unsigned long pump1_operationTime = 0;  // operating time for pump A in minutes
// unsigned long pump2_operationTime = 0;  // operating time for pump B in minutes
// int linkPump = 0;

// // physical pump is operating
// int pump1_op = 0;
// int pump2_op = 0;

// // logical pumpis operating
// int pumpA_op = 0; // indicator, that pumpA is running
// int pumpB_op = 0; // indicator, that pumpB is running

//*******************************************************************************
void measureOperatingTime(PumpStatus &pumpStatus, PumpControl &pumpControl)
//*******************************************************************************
{
    // local variables
    unsigned long opTime_millisNow = 0;
    unsigned long opTime_millisDiff = 0;
    unsigned long opTime_previousMillis = 0;

    // measure time since last invocation
    opTime_millisNow = millis();

    opTime_millisDiff = opTime_millisNow - opTime_previousMillis;

    if (opTime_millisDiff > timeUnit_opTime)
    {

        opTime_previousMillis = opTime_millisNow;

        if (pumpControl.pump1_op == 1)
        {
            // in case of pump A is running, add actual runtime to operationTime (millis * timUnit_opTime)
            pumpStatus.pump1_operationTime += opTime_millisDiff / timeUnit_opTime;
        }

        if (pumpControl.pump2_op == 1)
        {
            // in case of pump B is running, add actual runtime to operationTime (millis * timUnit_opTime)
            pumpStatus.pump2_operationTime += opTime_millisDiff / timeUnit_opTime;
        }
    }
}

//*******************************************************************************
void controlPump(PumpControl &pumpControl)
{
    //*******************************************************************************

    // local variables
    unsigned long start2ndPump_millisNow;
    unsigned long start2ndPumpNow = 0;
    unsigned long startPumpA_millis = 0;

    start2ndPump_millisNow = millis();

    // if A is running and B not we generate a start pulse for B after timeToSecondPump
    //  pulse is blocked when B is already running
    if ((((start2ndPump_millisNow - startPumpA_millis) / timeUnit_opTime) > timeToSecondPump) && (pumpControl.pumpA_op == 1) && (pumpControl.pumpB_op == 0))
    {
        start2ndPumpNow = 1;
    }

    // if waterlevel comes over bodenplatte start both pumps immediately
    if (alarmState >= 6)
    {
        pumpControl.pumpA_op = 1;
        pumpControl.pumpB_op = 1;
    }
    // if waterlevel too high and pumpA not running -> start first pump
    if ((alarmState >= 5) && (pumpControl.pumpA_op == 0))
    {
        pumpControl.pumpA_op = 1;
    }
    // if after the timeout we are still at alarmState 5, start second pump
    else if ((alarmState >= 5) && (start2ndPumpNow == 1))
    {
        pumpControl.pumpB_op = 1;
        start2ndPumpNow = 0; // only a pulse
    }
    else if (alarmState <= 3)
    { // waterlevel back to normal -> stop both pumps
        pumpControl.pumpA_op = 0;
        pumpControl.pumpB_op = 0;
    }

    // remember time, wenn pumpA is started
    if (pumpControl.pumpA_op == 0)
    {
        startPumpA_millis = millis();
    }
}

//*******************************************************************************
void selectPump(PumpStatus &pumpStatus, PumpControl &pumpControl)
{
    //*******************************************************************************
    // link logic pumpA/B to physical available pump1/2
    // the pump with lesser operating time is used as pumpA
    // the other pump
    // if the difference between the two operating times is too big, the pumps will be exchanged

    // select which pump has to run
    // linkPump =0;   // A->1 -- B->2
    // linkPump =1;   // A->2 -- B->1

    int pump_operationTimeDiff = (pumpStatus.pump1_operationTime -
                                  pumpStatus.pump2_operationTime);

    if ((pumpStatus.linkPump == 0) &&
        (abs(pump_operationTimeDiff) > opTimeToExchange) &&
        (pumpStatus.pump1_operationTime > pumpStatus.pump2_operationTime))
    {
        pumpStatus.linkPump = 1;
        putSetupIni(pumpStatus);
    }
    else if ((pumpStatus.linkPump == 1) &&
             (abs(pump_operationTimeDiff) > opTimeToExchange) &&
             (pumpStatus.pump1_operationTime <= pumpStatus.pump2_operationTime))
    {
        pumpStatus.linkPump = 0;
        putSetupIni(pumpStatus);
    }

    if (pumpStatus.linkPump == 0)
    {
        pumpControl.pump1_op = pumpControl.pumpA_op;
        pumpControl.pump2_op = pumpControl.pumpB_op;
    }
    else
    {
        pumpControl.pump2_op = pumpControl.pumpA_op;
        pumpControl.pump1_op = pumpControl.pumpB_op;
    }

    // set output pin to control pump
    digitalWrite(GPout_pump1, pumpControl.pump1_op);
    digitalWrite(GPout_pump2, pumpControl.pump2_op);
}
