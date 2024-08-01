/********************************************************************* */
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


// void showTime() {
//   tm tmL;
//   time(&epochTime);                       // read the current time
//   localtime_r(&epochTime, &tmL);           // update the structure tm with the current time
//   Serial.print("year:");
//   Serial.print(tmL.tm_year + 1900);  // years since 1900
//   Serial.print("\tmonth:");
//   Serial.print(tmL.tm_mon + 1);      // January = 0 (!)
//   Serial.print("\tday:");
//   Serial.print(tmL.tm_mday);         // day of month
//   Serial.print("\thour:");
//   Serial.print(tmL.tm_hour);         // hours since midnight  0-23
//   Serial.print("\tmin:");
//   Serial.print(tmL.tm_min);          // minutes after the hour  0-59
//   Serial.print("\tsec:");
//   Serial.print(tmL.tm_sec);          // seconds after the minute  0-61*
//   Serial.print("\twday");
//   Serial.print(tmL.tm_wday);         // days since Sunday 0-6
//   if (tmL.tm_isdst == 1)             // Daylight Saving Time flag
//     Serial.print("\tDST");
//   else
//     Serial.print("\tstandard");
//   Serial.println();
// }

// void showTimeAlternative() {
//   //time_t epochTime;                       // this are the seconds since Epoch (1970) - UTC
//   tm tmL;                            // the structure tm holds time information in a more convenient way
//   time(&epochTime);                       // read the current time
//   localtime_r(&epochTime, &tmL);           // update the structure tm with the current time
//   char buffer[42] {0};              // a buffer large enough to hold your output
//   strftime (buffer, sizeof(buffer), "%H:%M", &tmL);  // for different formats see https://cplusplus.com/reference/ctime/strftime/
//   Serial.println(buffer);
// }

// void getFormattedDateAndTime()
// {
//   //time_t epochTime;                       // this are the seconds since Epoch (1970) - UTC
//   tm tmL;                           // the structure tm holds time information in a more convenient way
//   time(&epochTime);                       // read the current time
//   localtime_r(&epochTime, &tmL);          // update the structure tm with the current time
  
//   char buffer[42] {0};              // a buffer large enough to hold your output
//   strftime (buffer, sizeof(buffer), "%H:%M:%S", &tmL);  // for different formats see https://cplusplus.com/reference/ctime/strftime/
//   formattedTime = buffer;

//   strftime (buffer, sizeof(buffer), "%F", &tmL);  // for different formats see https://cplusplus.com/reference/ctime/strftime/
//   currentDate = buffer;

// }

void getEpochTime(time_t &epochTime)
{
  time(&epochTime);                       // read the current time
}

void formatDateAndTime(String &formattedTime, String &formattedDate, time_t epochTime)
{
  //time_t epochTime;                       // this are the seconds since Epoch (1970) - UTC
  tm tmL;                           // the structure tm holds time information in a more convenient way
  localtime_r(&epochTime, &tmL);          // update the structure tm with the current time
  
  char buffer[42] {0};              // a buffer large enough to hold your output
  strftime (buffer, sizeof(buffer), "%H:%M:%S", &tmL);  // for different formats see https://cplusplus.com/reference/ctime/strftime/
  formattedTime = buffer;

  strftime (buffer, sizeof(buffer), "%F", &tmL);  // for different formats see https://cplusplus.com/reference/ctime/strftime/
  formattedDate = buffer;

}
