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

// variables and constants

unsigned long pumpA_operationTime = 0;  // operating time for pump A in minutes
unsigned long pumpB_operationTime = 0;  // operating time for pump B in minutes

int pumpA_op = 0;       // indicator, that pumpA is running
int pumpB_op = 1;       // indicator, that pumpB is running


unsigned long opTime_millisNow=0;
unsigned long opTime_millisDiff=0;
unsigned long opTime_previousMillis=0;

//*******************************************************************************

void measureOperatingTime()
{
    
    /*================================================================*/
    // measure time since last invocation
    /*================================================================*/
    opTime_millisNow = millis();
    
    opTime_millisDiff = opTime_millisNow - opTime_previousMillis;

    if (opTime_millisDiff > timeUnit_opTime) {

        opTime_previousMillis = opTime_millisNow;


        if (pumpA_op == 1)
        {
            // in case of pump A is running, add actual runtime to operationTime (millis * timUnit_opTime)
            pumpA_operationTime += opTime_millisDiff / timeUnit_opTime;
        
        }
        
        if (pumpB_op == 1)
        {
            // in case of pump B is running, add actual runtime to operationTime (millis * timUnit_opTime)
            pumpB_operationTime += opTime_millisDiff / timeUnit_opTime;
        
        }
    }

}

void controlPump() {

    // if waterlevel too high -> start first pump
    if (alarmState >= 5)
    {
        pumpA_op = 1;
    } else if (alarmState <= 3) { // waterlevel back to normal -> stop botz pumps
        pumpA_op = 0;
        pumpB_op = 0;
    }
}