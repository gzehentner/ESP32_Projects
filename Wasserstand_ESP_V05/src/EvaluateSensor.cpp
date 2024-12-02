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
#include <server.h>
#ifdef use_FS 
  #include <LittleFS.h> 
  #include <MyLittleFSLib.h>
#endif


/*=================================================================*/
/* Variables for CurrentLoop */
/*=================================================================*/

CurrentLoopSensor currentLoopSensor(sensorPin, resistor, vref, maxValue); // create the sensor object 

int    myValueFiltered        = 0; // result of the filtering
int    myValueFilteredAct     = 0; // actual result for display in web page
int    myValueFilteredAct_old = 0; // old value to identify change

int    myAdcFiltered    = 0;
int    myAdcFilteredAct = 0;

int    filterCnt    = 0;       // count loop runs for collecting values for filter

int valueStable = 0;           // value is not stable until the first filtering is done (so values should not be sent to website)


unsigned long millisDiff;
unsigned long longtermMillisDiff;

// variable for simulation
//    used to hold the actual waterlevel if simulation is switched on
float pegel      = Level_ALL - 5;  // waterlevel in cm

/*=================================================================*/
/* definitions for shortterm ring buffer */
/*=================================================================*/
unsigned long ringTime [iRingValueMax +1];
int    ringValue[iRingValueMax +1];     // ring buffer for display last 50 values
int    ringADC  [iRingValueMax +1];     // ring buffer for display last 50 adc values
int    wrRingPtr = 0;                  // ring buffer write pointer 
int    rdRingPtr = 0;                  // ring buffer read pointer 

/*=================================================================*/
/* definitions for longterm ring buffer */
/*=================================================================*/
unsigned long ringLongtermTime [iLongtermRingValueMax +1];
int    ringLongtermValue[iLongtermRingValueMax +1];     // ring buffer for display last 50 values
int    wrLongtermRingPtr = 0;                  // ring buffer write pointer 
int    rdLongtermRingPtr = 0;                  // ring buffer read pointer 

//int firstRun = 1;
int printOnChangeActive = 0;

/*=================================================================*/
// global variables for send mail
/*=================================================================*/
String subject="";
String htmlMsg="";

/* END Variables for CurrentLoop */
/*=================================================================*/


/*=================================================================*/
  void beginCurrentLoopSensor()
/*=================================================================*/
  {
    currentLoopSensor.begin(); // start the sensor 
//    currentLoopSensor.check();  // check the values and settings
  }

/*================================================================*/
/*   get analog values from currentLoopSensor and calculate level */
/*   store values in two ring buffers =                           */
/*================================================================*/
  void Current2Waterlevel()
/*================================================================*/
{
    /*================================================================*/
    // measure time and return, when to early
    /*================================================================*/
    millisNow = millis();
    
    millisDiff = millisNow - previousMillis;
    longtermMillisDiff = millisNow - longtermPreviousMillis;

    // when it is too early, do nothing
    if (millisDiff <= measureInterval) return; // do nothing
    previousMillis = millisNow;

    /*================================================================*/
    // if not too early, get values
    /*================================================================*/
    // in debug mode, variable pegel has to be used
    // pegel is set by function setPegelforSimulation() as direct value
    // in a coming version may be we set a PWM value and read back is done with the ADC
    if (debugLevelSwitches) {
      myValueFiltered += pegel;
    } else 
   /*================================================================*/
   // in normal mode values from ADC are used
    {
      /*================================================================*/
      myValueFiltered +=  currentLoopSensor.getValueUnfiltered(); // read sensor value into variable
      /*================================================================*/
                                                        // and create a sum for filtering
      myAdcFiltered   += currentLoopSensor.getAdc();    // read ADC raw value

    } 

    /*================================================================*/
    /*== increment filterCnt */
    filterCnt++;

    /*================================================================*/
    // when a new filtered value is calculated ()
    /*================================================================*/
    if (filterCnt >= filterCntMax)
    {
       
      // calculate average
      myValueFilteredAct = myValueFiltered / filterCntMax;     
      myAdcFilteredAct   = myAdcFiltered   / filterCntMax;

      /*=========================================*/
      // value is not stable until the first filtering is done
      valueStable = 1;

      /*=========================================*/
      /*=========================================*/
      /* handle shortterm values */
      /*=========================================*/

      // write time to ring buffer
      ringTime [wrRingPtr] = epochTime; // myEpochTime;

      // write shortterm value to ring buffer
      ringValue[wrRingPtr] = round(myValueFilteredAct);  

      // add ADC raw values to ring buffer
      ringADC[wrRingPtr]  = myAdcFilteredAct;

      // increment write pos
      if (wrRingPtr<iRingValueMax) {
        wrRingPtr++;
      } else  {
        wrRingPtr = 0;
      }
      
      //==============================================
      // write data to logfile (but only if value has changed)
      //==============================================
      int diffMyValue = (myValueFilteredAct - myValueFilteredAct_old);
      if (abs(diffMyValue)>1) { // print on change only (change > 1 to avoid too often change)
        printOnChangeActive = 1;

        String tempString = "";
        tempString += epochTime;
        tempString += ", ";
        tempString += myValueFilteredAct;
        
        #ifdef use_FS
        appendFile("/level.log", (tempString+ "\n").c_str()); // Append data to the file
        #endif
        myValueFilteredAct_old = myValueFilteredAct;

      } // end of print on change

      /*=========================================*/
      /*=========================================*/
      /* handle longterm values */
      /*=========================================*/
      if (longtermMillisDiff > longtermInterval) 
      {
        longtermPreviousMillis = millisNow;

        // write time to ring buffer
        ringLongtermTime [wrLongtermRingPtr] = epochTime; // myEpochTime;

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
      filterCnt = 0; 

      /*================================================================*/
      // end handle new filter value
    }
  }

  /*=====================================================*/
  /*
    - read out relais status
    - set alarm state accordingly
  */
  /*=====================================================*/
  void SetAlarmState_from_relais() {
  /*=====================================================*/
    //++++++++++++++++++++++
    // Read in relais status
    //++++++++++++++++++++++
    val_AHH = digitalRead(GPin_AHH);
    val_AH =  digitalRead(GPin_AH);
    val_AL =  digitalRead(GPin_AL);
    // val_ALL = digitalRead(GPin_ALL);
    

    //++++++++++++++++++++++
    // set alarmStateRelais
    //++++++++++++++++++++++
    alarmStateRelaisOld = alarmStateRelais;
  
    if ((val_AHH == 0) && (val_AH == 0))
    {
      alarmStateRelais = 5;       // Alarm, pump running
    }
    else if (val_AH == 0)
    {
      alarmStateRelais = 4;       // severe warning
    }
    else if (val_AL == 1)
    {
      alarmStateRelais = 3;       // normal high
    }
    else if ((val_AL == 0) ) //&& (val_ALL == 1))
    {
      alarmStateRelais = 2;       // normal
    }
    // else if ((val_ALL == 0))
    // {
    //   alarmStateRelais = 1;    // normal lo9w
    // }

    }

  /*=====================================================*/
  /*
    - compare measured waterlevel with alarm level
    - set alarm state
  */
  /*=====================================================*/
  void SetAlarmState_from_level() {
  /*=====================================================*/

    
    //++++++++++++++++++++++
    // set alarmStateLevel
    //++++++++++++++++++++++
    alarmStateLevelOld = alarmStateLevel;
  
    // --------------------------------------------------------
    // --- 6 --------------------------------------------------
    // --------------------------------------------------------
    // switch immediately
    if ((alarmStateLevel<6) && (myValueFilteredAct > Level_AHHH))
    {
      alarmStateLevel = 6;       // 2x pump running imediately
    }
    // switch with hysteresis
    else if ((alarmStateLevel>=6) && (myValueFilteredAct > Level_AHHH-Level_HIST))
    {
      alarmStateLevel = 6;       // 2x pump running imediately
      
    }
    // --------------------------------------------------------
    // --- 5 --------------------------------------------------
    // --------------------------------------------------------
    // switch immediately
    else if ((alarmStateLevel<5) && (myValueFilteredAct > Level_AHH))
    {
      alarmStateLevel = 5;       // pump running
    }
    // switch with hysteresis
    else if ((alarmStateLevel=5) && (myValueFilteredAct > Level_AHH-Level_HIST))
    {
      alarmStateLevel = 5;       // pump running
    }
    // --------------------------------------------------------
    // --- 4 --------------------------------------------------
    // --------------------------------------------------------
    // switch immediately
    else if ((alarmStateLevel<4) && (myValueFilteredAct > Level_AH))
    {
      alarmStateLevel = 4;       // severe warning
    }
    // switch with hysteresis
    else if ((alarmStateLevel>=4) && (myValueFilteredAct > Level_AH-Level_HIST))
    {
      alarmStateLevel = 4;       // severe warning
    }
    // --------------------------------------------------------
    // --- 3 --------------------------------------------------
    // --------------------------------------------------------
    // switch immediately
    else if ((alarmStateLevel<3) && (myValueFilteredAct > Level_AL))
    {
      alarmStateLevel = 3;       // normal high
    }
    // switch with hysteresis
    else if ((alarmStateLevel>=3) && (myValueFilteredAct > Level_AL-Level_HIST))
    {
      alarmStateLevel = 3;       // normal high
    }
    // --------------------------------------------------------
    // --- 2 --------------------------------------------------
    // --------------------------------------------------------
    else 
    {
      alarmStateLevel = 2;       // normal low
    }
  }

  /*=====================================================*/
  /*
    - compare the two alarm levels and set the global alarm state to worst case
    - set alarm state
  */
  /*=====================================================*/
  void calculateWorstCaseAlarmState()
  {
    alarmStateOld = alarmState;

    if (alarmStateLevel > alarmStateRelais) {
      alarmState = alarmStateLevel;
    } else {
      alarmState = alarmStateRelais;
    }
    
    if ((alarmStateRelais != alarmStateRelaisOld) || (alarmStateLevel != alarmStateLevelOld)) {
      Serial.print("alarmStateRelais: "); Serial.println(alarmStateRelais);
      // Serial.print("AHH: "); Serial.println(val_AHH);
      // Serial.print("AH : "); Serial.println(val_AH);
      // Serial.print("AL : "); Serial.println(val_AL);  
      Serial.print("alarmStatePegel: "); Serial.println(alarmStateLevel);
      Serial.print("Pegel: ");Serial.println(pegel);
    }
  }
  /******************************************************** */
  /* prepare send mail depending on alarmState */
  /******************************************************** */
  void prepareSendMail ()
  /******************************************************** */
  {
    // reserve enough space for strings
    subject.reserve(50);
    htmlMsg.reserve(200);

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
        else if (alarmState == 2)
        {
          // info that level is now ok
          // Serial.println(F("level decreased to Low"));
          // subject = F("Pegel Zehentner -- sehr niedrig ");
          // htmlMsg = F("<p>Wasserstand Zehentner ist sehr niedrig. Alles OK</p>");
          // executeSendMail = true;
        }
      }
      else if (alarmStateOld == alarmState)
      {
        // do nothing
        executeSendMail = false;
      }
    }
  }


/************************************
 * setPegelforSimulation
 * * set value of pegel direct by this function
 * * depending on the alarmState we set a value +5cm or -5cm to the
 *     value of the Level_Axx
 * ************************************** */
void setPegelforSimulation()
{
  if ((simVal_AHH==0) and (simVal_AH==0)) {
    pegel = Level_AHH + 5;  // for simulation set pegel to Alarm-Level plus 5cm
  } else if ((simVal_AH==0)) {
    pegel = Level_AH  + 5;  // for simulation set pegel to Warning-Level plus 5cm
  } else if (simVal_AL==1) {
    pegel = Level_AL + 5;  // for simulation set pegel to OK-Level 1 plus 5cm
  } else if (simVal_AL==0) {
    pegel = Level_AL - 5;  // for simulation set pegel to OK-Level minus 5cm
  }
}
/*================================= 
  calculate PWM duty cycle for simulation of analog value 
  ==================================
  representing level in mm
  
  Input: level Waterlevel [in mm]
  Return: PWM duty cycle as number with 255 = 100%
  
  */
// not used yet; needs some care
// float Waterlevel2dutyCycle (float level) {
// 
//   const float maxPegel =  5.0;  // m
//   const float Imax     = 20.0; // mA
//   const float Imin     =  4.0; // mA
// 
//   float Vact     =  0.0;  // actual voltage
//   float Vmax     =  0.0;  // voltage at max Pegel
// 
//   Vmax = (((Imax-Imin))               +Imin)/1000.0*resistor;
//   
//   Vact = (((Imax-Imin)/maxPegel*level)+Imin)/1000.0*resistor;;
// 
//   // Serial.print("Vact: ");Serial.println(Vact);
//   // Serial.print("Vmax: ");Serial.println(Vmax);
//   
//   return Vact / Vmax*255.0;
// }