/********************************************************************* */
/********************************************************************* */
/* simply way to get current time and date */
/********************************************************************* */


#ifndef TIMESERVER_H
#define TIMESERVER_H

/* Configuration of NTP */
#define MY_NTP_SERVER "at.pool.ntp.org"           
#define MY_TZ "CET-1CEST,M3.5.0/02,M10.5.0/03"  


// declare global variables
extern String currentDate;   // hold the current date
extern String formattedTime; // hold the current time

extern time_t epochTime;

void getEpochTime(time_t &epochTime);
void formatDateAndTime(String &formattedTime, String &formattedDate, time_t epochTime);


#endif