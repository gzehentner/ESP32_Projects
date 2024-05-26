

#ifndef TIMESERVER_H
#define TIMESERVER_H

// declare global variables
extern String currentDate;   // hold the current date
extern String formattedTime; // hold the current time
//extern unsigned long myEpochTime; 

extern time_t epochTime;
// extern tm  tmX;

void getEpochTime(time_t &epochTime);
void formatDateAndTime(String &formattedTime, String &formattedDate, time_t epochTime);


#endif