/*
 * TempLogger.h
 *
 *
 * Temperature Logger class for the ESP 32
 * This is a TEST file to find the bug in my logger
 *
 *
 *  Created on: 20.12.2022
 *      Author: KaiAtHome
 */

#ifndef TEMPLOGGER_H_
#define TEMPLOGGER_H_

#include <Arduino.h>
#include "SignalStatistics.h"

#include "KBuffer.h"
#include "DHTesp.h" // lib for the DHT22 sensor
#include "SHT31.h"


#define SHT31_ADDRESS   0x44
#define SHT_DATA_PIN 21
#define SHT_CLOCK_PIN 22

#define NO_OF_LOGS	2		  // number of logs with individual data queues. Here: 1 long term log, 1 short term log
#define NO_OF_STORE_VALS_SHORT_TERM 5000 // number of buffered values
#define NO_OF_STORE_VALS_LONG_TERM 1000 // number of buffered values

enum enSensorType{SensorSHT31, SensorDallas, SensorDHT22, SensorDummySim};
enum enSensorError{SensorErrorNone = 0, SensorErrorTimeout, SensorErrorChecksum};
enum enCurrentLog{LogShortTerm = 0, LogLongTerm};
enum enDataQueueType{DataTempVals = 0, DataHumidityVals};

class TempLogger
{
public:
	TempLogger();
	virtual ~TempLogger();

	//int Init(DallasTemperature *TempSensor);
	int MainLoop();
	int Init(DHTesp *myTempSensor);
	int Init(SHT31 *TempSensor);
	float GetTemperature();
	float GetHumidity();
	long GetTime();
	bool SetCurrentLog(enCurrentLog myCurrentLog);
	String GetErrorMessage();
	bool ResetBuffers();
	KBuffer <float> * GetDataQueue(enDataQueueType DataQueueType, enCurrentLog LogType);
	KBuffer <unsigned long> * GetTimeQueue(enCurrentLog LogType);

	/*
	KBuffer <float> *TempQueue;
	KBuffer <float> *HumidityQueue;
	KBuffer <unsigned long> *TimeQueue;	// time values at which temperature was measured
	*/
	KBuffer <float> TempQueue[NO_OF_LOGS];
	KBuffer <float> HumidityQueue[NO_OF_LOGS];
	KBuffer <unsigned long> TimeQueue[NO_OF_LOGS];	// time values at which temperature was measured
private:
	SHT31 *TempHumSensorSHT;
	DHTesp *TempHumSensorDHT;
	enCurrentLog CurrentLog;
	enSensorType SensorType;
	enSensorError SensorError;
	String SensorStatusString;
	float Humidity, Temperature;
	String HumidityStr, TemperatureStr;
	String ErrorStr;

	SignalStatistics StatHumidity[NO_OF_LOGS], StatTemperature[NO_OF_LOGS];
	unsigned long int myTime, lastTimeSensor, dTSensor;
	unsigned long int timeLastShortTermDataStored_k1, timeLastLongTermDataStored_k1, TShortTermDataStoreInMin, TLongTermDataStoreInHour;
	time_t CurrentTimeNum;	// current time as a single value

};

#endif /* TEMPLOGGER_H_ */
