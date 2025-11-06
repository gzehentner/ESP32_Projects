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
#include <server.h>
#include <MyLittleFSLib.h>

// variables and constants

// nightly check
// global: merkt sich den letzten ausgelösten Tag (day of year)
int lastTriggeredYDay = -1;

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
    static unsigned long opTime_millisNow_mea = 0;
    static unsigned long opTime_millisDiff_mea = 0;
    static unsigned long opTime_previousMillis_mea = 0;

    // measure time since last invocation
    opTime_millisNow_mea = millis();

    opTime_millisDiff_mea = opTime_millisNow_mea - opTime_previousMillis_mea;

    if (opTime_millisDiff_mea > timeUnit_opTime)
    {

        opTime_previousMillis_mea = opTime_millisNow_mea;

        if (pumpControl.pump1_op == 1)
        {
            // in case of pump A is running, add actual runtime to operationTime (millis * timUnit_opTime)
            pumpStatus.pump1_operationTime += opTime_millisDiff_mea / timeUnit_opTime;
        }

        if (pumpControl.pump2_op == 1)
        {
            // in case of pump B is running, add actual runtime to operationTime (millis * timUnit_opTime)
            pumpStatus.pump2_operationTime += opTime_millisDiff_mea / timeUnit_opTime;
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
    static unsigned long startPumpA_millis = 0;

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

/*=================================================================*/
// einmalig TZ setzen (CET = GMT+1 mit DST)
void initTimeZone()
/*=================================================================*/
{
    // Option A: ESP32-empfohlen (nutzt NTP und TZ)
    configTzTime("CET-1CEST,M3.5.0/2,M10.5.0/3", "pool.ntp.org", "time.nist.gov");

    // Option B: einfache TZ-Setzung (setzt nur Lokale Zeitumrechnung)
    // setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1);
    // tzset();
}

//*******************************************************************************
void checkPump(PumpStatus &pumpStatus, PumpControl &pumpControl, int triggerCheckPump,
unsigned long dailyPumpRunTime)

//*******************************************************************************
// timer controlled check of pump
// run each pump every day 1 o'clock for 10s
//   one after the other
{
    static unsigned long previousMillis = 0;
    unsigned long millisDiff;
    static int pumpCheckPause = 0; 

    if (triggerCheckPump == 1) {
        Serial.println("trigger check pump");

        //************************************************************************* */
        // write to file
        String errMessage = "";
        errMessage = currentDate;
        errMessage += " - ";
        errMessage += formattedTime;
        errMessage += " - ";
        errMessage += " daily check pump triggered";
        errMessage += "\n";
        appendFile("/error.log", errMessage.c_str());
        //************************************************************************* */
        triggerCheckPump = 0;

        //---------------------------------
        // Start Pump1 immediately
        pumpControl.checkPump1 = 1;
        pumpCheckPause = 0;
        previousMillis = millis();
    }

    /*================================================================*/
    // measure time and return, when to early
    /*================================================================*/
    millisDiff = millis() - previousMillis;

    // when it is too early, do nothing
    if (millisDiff > dailyPumpRunTime)
    {
        previousMillis = millis();

        // after time stop Pump1 and enter pause
        if (pumpControl.checkPump1== 1)
        {
            Serial.println("Check 1 ready");
            pumpControl.checkPump1 = 0;
            pumpCheckPause = 1;
        }
        // after pause start Pump2
        else if (pumpCheckPause == 1)
        {
            Serial.println("Check pause");
            pumpCheckPause = 0;
            pumpControl.checkPump2 = 1;
        }
        // Finised
        else if (pumpControl.checkPump2 == 1)
        {
            Serial.println("Check 2 ready");
            pumpControl.checkPump2 = 0;
        }
    }
}

/*=================================================================*/
// prüfen und einmal täglich um 01:00 auslösen
void checkDailyTrigger(PumpStatus &pumpStatus, PumpControl &pumpControl,
                int &triggerCheckPump)

/*=================================================================*/
{
    time_t now = time(nullptr); // epoch in UTC
    struct tm timeinfo;
#if defined(ARDUINO_ARCH_ESP32)
    localtime_r(&now, &timeinfo); // wandelt mit gesetzter TZ in lokale Zeit
#else
    struct tm *tmp = localtime(&now);
    if (!tmp)
        return;
    timeinfo = *tmp;
#endif

#ifdef DEBUG_DAILY_TRIGGER

            if (timeinfo.tm_sec == 0) // Testbetrieb sec == 0 -> jede Minute
            {
                if (lastTriggeredYDay != timeinfo.tm_min) // Testbetrieb
                {
                    lastTriggeredYDay = timeinfo.tm_min;

#else
    if (timeinfo.tm_hour == 1 && timeinfo.tm_min == 0) // Stunde 1 - Min 0 -> (01:00)
    {
        if (lastTriggeredYDay != timeinfo.tm_yday)
        {
            lastTriggeredYDay = timeinfo.tm_yday;
#endif                    
                    ////
                    triggerCheckPump = 1;
                    Serial.println("Nightly action at 01:00");
                    ////
                } else {
                    triggerCheckPump = 0;
                }
            } else {
                triggerCheckPump = 0;
            }
        }

        //*******************************************************************************
        void manualPumpOperation(PumpControl & PumpControl)
        {
            // when manual control is active, automatic control is disabled
            if (PumpControl.manualPumpControl == 1)
            {
                digitalWrite(GPout_pump1, PumpControl.manPump1Enabled);
                digitalWrite(GPout_pump2, PumpControl.manPump2Enabled);
            }
            else if ((PumpControl.checkPump1 == 1) or (PumpControl.checkPump2 == 1))
            {
                // set output pin to control pump
                digitalWrite(GPout_pump1, PumpControl.checkPump1);
                digitalWrite(GPout_pump2, PumpControl.checkPump2);
            }
            else
            {
                // set output pin to control pump
                digitalWrite(GPout_pump1, PumpControl.pump1_op);
                digitalWrite(GPout_pump2, PumpControl.pump2_op);
            }
        }

        //*******************************************************************************
        void selectPump(PumpStatus & pumpStatus, PumpControl & pumpControl)
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
            // digitalWrite(GPout_pump1, pumpControl.pump1_op);
            // digitalWrite(GPout_pump2, pumpControl.pump2_op);
        }
