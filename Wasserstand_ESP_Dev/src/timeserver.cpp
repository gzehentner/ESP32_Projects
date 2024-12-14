/********************************************************************* */
/********************************************************************* */
/* simply way to get current time and date */
/********************************************************************* */

#include <Arduino.h>
#include "time.h"                   // for time() ctime()

/*=================================================================*/
/* ===========   prepare timeserver =================*/


/* Globals */
time_t epochTime;                         // this are the seconds since Epoch (1970) - UTC

// declare global variables
String currentDate;   // hold the current date
String formattedTime; // hold the current time


/*=================================================================*/
void getEpochTime(time_t &epochTime)
/*=================================================================*/
{
  time(&epochTime);                       // read the current time
}

/*=================================================================*/
void formatDateAndTime(String &formattedTime, String &formattedDate, time_t epochTime)
/*=================================================================*/
{
  tm tmL;                           // the structure tm holds time information in a more convenient way
  localtime_r(&epochTime, &tmL);    // update the structure tm with the current time
  
  char buffer[42] {0};              // a buffer large enough to hold your output
  strftime (buffer, sizeof(buffer), "%H:%M:%S", &tmL);  // for different formats see https://cplusplus.com/reference/ctime/strftime/
  formattedTime = buffer;

  strftime (buffer, sizeof(buffer), "%F", &tmL);  // for different formats see https://cplusplus.com/reference/ctime/strftime/
  formattedDate = buffer;

}
