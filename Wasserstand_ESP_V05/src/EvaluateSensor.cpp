//* *******************************************************************
//  Current2Waterlevel
// 
//  Use Currentloop to measure the analog value
//  Convert to Waterlevel
//* *******************************************************************

#include <Arduino.h>
#include <timeserver.h>
#include <waterlevel_defines.h>
#include <waterlevel.h>
#include <NoiascaCurrentLoop.h>   // library for analog measurement

/*=================================================================*/
/* Parameter for CurrentLoop */
const byte sensorPin    = Ain_Level;  // ADC pin for the sensor
const uint16_t resistor = 165;        // used shunt  resistor in Ohm
const byte vref         = 32;         // VREF in Volt*10 (Uno 16MHz: 50, ProMini 8MHz: 3V3).
const int maxValue      = 5000;        // measurement range: 000cm to 5000mm --> maxValue=5000


/*=================================================================*/
/* Variables for CurrentLoop */

CurrentLoopSensor currentLoopSensor(sensorPin, resistor, vref, maxValue); // create the sensor object 

int    myValueFiltered  = 0;   // result of the filtering
int    myValueFilteredAct = 0; // actual result for display in web page
int    myAdcFiltered    = 0;

int    filterCnt    = 0;       // count loop runs for collecting values for filter

const int  measureInterval = 100; // measurement interval in milliseconds


unsigned long millisDiff;
unsigned long longtermMillisDiff;

// shortterm 
unsigned long ringTime [iRingValueMax +1];
int    ringValue[iRingValueMax +1];     // ring buffer for display last 50 values
// int    ringADC  [iRingValueMax +1];     // ring buffer for display last 50 adc values
int    wrRingPtr = 0;                  // ring buffer write pointer 
int    rdRingPtr = 0;                  // ring buffer read pointer 

// longterm 
unsigned long ringLongtermTime [iLongtermRingValueMax +1];
int    ringLongtermValue[iLongtermRingValueMax +1];     // ring buffer for display last 50 values
int    wrLongtermRingPtr = 0;                  // ring buffer write pointer 
int    rdLongtermRingPtr = 0;                  // ring buffer read pointer 

//int firstRun = 1;

// global variables for send mail
String subject="";
String htmlMsg="";



/* END Variables for CurrentLoop */
/*=================================================================*/

  void beginCurrentLoopSensor()
  {
    currentLoopSensor.begin(); // start the sensor 
//    currentLoopSensor.check();  // check the values and settings
  }

/*=====================================================*/
  void Current2Waterlevel()
{
        // measure time and return, when to early
    millisNow = millis();
    
    millisDiff = millisNow - previousMillis;
    longtermMillisDiff = millisNow - longtermPreviousMillis;

    // when it is too early, do nothing
    if (millisDiff <= measureInterval) return; // do nothing

    previousMillis = millisNow;

    // calculate average in interval: filterCntMax * 100us
    if (filterCnt < filterCntMax) {
      filterCnt++;
    #ifdef SIM_FADING_LEVEL
      myValueFiltered += pegel*1000;
    #else
      myValueFiltered +=  currentLoopSensor.getValueUnfiltered(); // read sensor value into variable
                                                        // and create a sum for filtering
    #endif
      myAdcFiltered   += currentLoopSensor.getAdc();    // read ADC raw value
    } else {
      // else: when a new filtered value is calculated ()
      filterCnt = 0;
  
    // calculate average
    myValueFilteredAct = myValueFiltered / filterCntMax;     
    
    /*=========================================*/
    /*=========================================*/
    /* handle shortterm values */
    /*=========================================*/

    // write time to ring buffer
    ringTime [wrRingPtr] = now; // myEpochTime;

    // write shortterm value to ring buffer
    ringValue[wrRingPtr] = myValueFilteredAct;  

    // add ADC raw values to ring buffer
    // ringADC[wrRingPtr]  = myAdcFiltered / filterCntMax;

    // increment write pos
    if (wrRingPtr<iRingValueMax) {
      wrRingPtr++;
    } else  {
      wrRingPtr = 0;
    }

    /*=========================================*/
    /*=========================================*/
    /* handle longterm values */
    /*=========================================*/
    if (longtermMillisDiff > longtermInterval) 
    {
      longtermPreviousMillis = millisNow;

      // write time to ring buffer
      ringLongtermTime [wrLongtermRingPtr] = now; // myEpochTime;

      // write shortterm value to ring buffer
      ringLongtermValue[wrLongtermRingPtr] = myValueFilteredAct;  

      // increment write pos
      if (wrLongtermRingPtr < iLongtermRingValueMax) {
        wrLongtermRingPtr++;
      } else  {
        wrLongtermRingPtr = 0;
      }
    }



    /*=========================================*/
    myValueFiltered = 0;
    myAdcFiltered   = 0;
  }
  }

  /*=====================================================*/
  /*
    - read out relais status
    - set alarm stat
    - prepare email text
  */
  void SetAlarmState_from_relais() {
  /*=====================================================*/

    // reserve enough space for strings
    subject.reserve(50);
    htmlMsg.reserve(200);

    //++++++++++++++++++++++
    // Read in relais status
    //++++++++++++++++++++++
    val_AHH = digitalRead(GPin_AHH);
    val_AH =  digitalRead(GPin_AH);
    val_AL =  digitalRead(GPin_AL);
    val_ALL = digitalRead(GPin_ALL);

    //++++++++++++++++++++++
    // set alarmState
    //++++++++++++++++++++++
    alarmStateOld = alarmState;
  
    if ((val_AHH == 0) && (val_AH == 0))
    {
      alarmState = 5;
    }
    else if (val_AH == 0)
    {
      alarmState = 4;
    }
    else if (val_AL == 1)
    {
      alarmState = 3;
    }
    else if ((val_AL == 0) && (val_ALL == 1))
    {
      alarmState = 2;
    }
    else if ((val_ALL == 0))
    {
      alarmState = 1;
    }
  
    //++++++++++++++++++++++
    // prepare send mail depending on alarmState
    //++++++++++++++++++++++
    if (alarmStateOld > 0)
    { // alarmStateOld == 0 means, it is the first run / dont send mail at the first run
      if (alarmStateOld < alarmState)
      { // water level is increasing
        if (alarmState == 4)
        {
          // send warning mail
          Serial.println(F("warning mail should be sent"));
          subject = F("Pegel Zehentner -- Warnung ");
          htmlMsg = F("<p>Wasserstand Zehentner ist in den Warnbereich gestiegen <br>");
          htmlMsg += F("Pegelstand über die Web-Seite: </p>; // <a href='http://zehentner.dynv6.net:400'>Wasserstand-Messung</a> beobachten </p>");
          executeSendMail = true;
        }
        else if (alarmState == 5)
        {
          // send alarm mail
          Serial.println("alarm mail should be sent");
          subject = F("Pegel Zehentner -- Alarm ");
          htmlMsg = F("<p>Wasserstand Zehentner ist jetzt im Alarmbareich<br>");
          htmlMsg += F("es muss umgehend eine Pumpe in Betrieb genommen werden. <br>");
          htmlMsg += F("Pegelstand über die Web-Seite: <a href='http://zehentner.dynv6.net:400'>Wasserstand-Messung</a> beobachten </p>");
          executeSendMail = true;
        }
      }
      else if (alarmStateOld > alarmState)
      { // water level is decreasing
        if (alarmState == 4)
        {
          // info that level comes from alarm and goes to warning
          Serial.println(F("level decreasing, now warning"));
          subject = F("Pegel Zehentner -- Warnung ");
          htmlMsg = F("<p>Wasserstand Zehentner ist wieder zurück in den Warnbereich gesunken<br>");
          htmlMsg += F("Pegelstand über die Web-Seite: <a href='http://zehentner.dynv6.net:400'>Wasserstand-Messung</a> beobachten </p>");
          executeSendMail = true;
        }
        else if (alarmState == 3)
        {
          // info that level is now ok
          Serial.println(F("level decreased to OK"));
          subject = F("Pegel Zehentner -- OK ");
          htmlMsg = F("<p>Wasserstand Zehentner ist wieder im Normalbereich</p>");
          executeSendMail = true;
        }
      }
      else if (alarmStateOld == alarmState)
      {
        // do nothing
        executeSendMail = false;
      }
    }
  }


/*================================= 
  calculate PWM duty cycle for simulation of analog value 
  ==================================
  representing level in mm
  
  Input: level Waterlevel [in mm]
  Return: PWM duty cycle as number with 255 = 100%
  
  */
float Waterlevel2dutyCycle (float level) {

  const float maxPegel =  5.0;  // m
  const float Imax     = 20.0; // mA
  const float Imin     =  4.0; // mA

  float Vact     =  0.0;  // actual voltage
  float Vmax     =  0.0;  // voltage at max Pegel

  Vmax = (((Imax-Imin))               +Imin)/1000.0*resistor;
  
  Vact = (((Imax-Imin)/maxPegel*level)+Imin)/1000.0*resistor;;

  // Serial.print("Vact: ");Serial.println(Vact);
  // Serial.print("Vmax: ");Serial.println(Vmax);
  
  return Vact / Vmax*255.0;
}