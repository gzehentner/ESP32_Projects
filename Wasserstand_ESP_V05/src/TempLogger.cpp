/*
 * TempLogger.cpp
 *
 * Temperature Logger class for the ESP 32
 * This is a TEST file to find the bug in my logger
 *
 *
 *  Created on: 20.12.2022
 *      Author: KaiAtHome
 */

#include "TempLogger.h"

/*==========================================================================================================================
 * Default constructor of the TempLogger class
 *
 *
 *
 *
 */
TempLogger::TempLogger():TempQueue({NO_OF_STORE_VALS_SHORT_TERM, NO_OF_STORE_VALS_LONG_TERM}), HumidityQueue({NO_OF_STORE_VALS_SHORT_TERM, NO_OF_STORE_VALS_LONG_TERM}),TimeQueue({NO_OF_STORE_VALS_SHORT_TERM, NO_OF_STORE_VALS_LONG_TERM})
{
	/*
	TempQueue = new KBuffer <float>[NO_OF_LOGS];
	HumidityQueue = new KBuffer <float>[NO_OF_LOGS];
	TimeQueue = new KBuffer <unsigned long>[NO_OF_LOGS];

	TempQueue[LogShortTerm] = KBuffer <float>(NO_OF_STORE_VALS_SHORT_TERM);
	TempQueue[LogLongTerm] = KBuffer <float>(NO_OF_STORE_VALS_LONG_TERM);
	HumidityQueue[LogShortTerm] = KBuffer <float>(NO_OF_STORE_VALS_SHORT_TERM);
	HumidityQueue[LogLongTerm] = KBuffer <float>(NO_OF_STORE_VALS_LONG_TERM);
	TimeQueue[LogShortTerm] = KBuffer <unsigned long>(NO_OF_STORE_VALS_SHORT_TERM);
	TimeQueue[LogLongTerm] = KBuffer <unsigned long>(NO_OF_STORE_VALS_LONG_TERM);
*/

	Temperature = 0;
	Humidity = 0;
	dTSensor = 2000; // time in ms between 2 sensor readings
	TShortTermDataStoreInMin = 3; // store a sensor reading every TShortTermDataStoreInMin minutes (debugging: in seconds!)
	TLongTermDataStoreInHour = 6; // store a sensor reading every TLongTermDataStoreInHour hours (!)
	timeLastShortTermDataStored_k1 = 0;
	timeLastLongTermDataStored_k1 = 0;
	lastTimeSensor = 0;
	myTime = 0;
	ErrorStr = ""; // document error states to display them in the webserver
	SensorType = SensorDHT22; // SensorDummySim;
	CurrentTimeNum = (time_t)0;
}

TempLogger::~TempLogger()
{

}


/*==========================================================================================================================
 * Initialize the SHT31 sensor and the TempLogger
 */
int TempLogger::Init(SHT31 *myTempSensor)
{
	TempHumSensorSHT = myTempSensor; // store pointer to external sensor object
	// Start the SHT31 temperature & humidity sensor
	Serial.println("Initialisiere SHT31");
	TempHumSensorSHT->begin(SHT31_ADDRESS, SHT_DATA_PIN, SHT_CLOCK_PIN);
	unsigned short int stat = TempHumSensorSHT->readStatus();
	Serial.print(stat, HEX);
	Serial.println();
	SensorType = SensorSHT31;
	return (1);
}



/*==========================================================================================================================
 * Initialize the DHT22 sensor and the TempLogger
 */
int TempLogger::Init(DHTesp *myTempSensor)
{
	TempHumSensorDHT = myTempSensor; // store pointer to external sensor object
	// Start the DHT22 temperature & humidity sensor
	Serial.println("Initialisiere DHT22");
	SensorType = SensorDHT22;
	return (1);
}
/*==========================================================================================================================
 * Main event loop of the TempLogger
 *
 */
int TempLogger::MainLoop()
{
	struct tm CurrentTime;
	char TimeStr[80];
	TempAndHumidity TempHumVal;
	int i;

	myTime = millis();

	//  sensor only in time intervals dTSensor, faster is not possible
	if ((myTime - lastTimeSensor) > dTSensor)
	{

		// Get the current system time used for logging
		if(getLocalTime(&CurrentTime))
		{
			CurrentTimeNum = mktime(&CurrentTime);
			Serial.print(&CurrentTime, "Time: %A, %B %d %Y %H:%M:%S   / ");
			strftime(TimeStr, 80, "%T", &CurrentTime);
		}


		//Serial.println("reading a value!");

		lastTimeSensor = myTime;

		// Read out the sensor based on the sensor type
		if(SensorType == SensorSHT31)
		{
			TempHumSensorSHT->read(true);	//default = true/fast       slow = false
			Temperature = TempHumSensorSHT->getTemperature(); 	// temperature from SHT31
			Humidity = TempHumSensorSHT->getHumidity(); 		//  humidity from SHT31
			SensorError = SensorErrorNone; // @@ TO DO:
			SensorStatusString = "";
		}

		if(SensorType == SensorDHT22)
		{
			TempHumVal = TempHumSensorDHT->getTempAndHumidity();// read temperature and humidity from DHT
			Temperature = TempHumVal.temperature;  				// with a single hardware access!
			Humidity  = TempHumVal.humidity;
			SensorError = (enSensorError) TempHumSensorDHT->getStatus();
			if(SensorError != SensorErrorNone)
			{
				SensorStatusString = String(TempHumSensorDHT->getStatusString());
			}
			else
			{
				SensorStatusString = "";
			}
		}
		if(SensorType == SensorDummySim)
		{
			// Simulate temperature & humidity
			Temperature = 20.0 + 10.0*sin((double)myTime/1e6); //
			Humidity  = 50.0 + 30.0*sin((double)myTime/3e6); //

		}
		// Check if the temperature and humidity seem reasonable
		if(ErrorStr.length() < 300)
		{
			if(SensorError != SensorErrorNone)
			{
				ErrorStr += "Status:" + SensorStatusString + " um " + String(TimeStr) + " / ";
			}
			if((Temperature < -5.0 )|| (Temperature > 45)  || isnan(Temperature))
			{
				ErrorStr +=  "Fehler Temperatur: " + String(Temperature) + " um " + String(TimeStr) + " / ";
			}
			if((Humidity < 10 )|| (Humidity > 90)|| isnan(Humidity))
			{
				ErrorStr = ErrorStr + "Fehler Humidity: " + String(Humidity) +  " um " + String(TimeStr)  + " / ";
			}
		}

		uint32_t FreeHeap, MinFreeHeap, MaxAllocHeap;

		// get the heap sizes BEFORE constructing the error message for the log server
		FreeHeap = ESP.getFreeHeap();
		MinFreeHeap = ESP.getMinFreeHeap();
		MaxAllocHeap = ESP.getMaxAllocHeap();

		/*
		Serial.print("FreeHeap = ");
		Serial.print(FreeHeap);
		Serial.print(" / MinFreeHeap = ");
		Serial.print(MinFreeHeap);
		Serial.print(" / MaxAllocHeap = ");
		Serial.println(MaxAllocHeap);


		Serial.print("T = ");
		Serial.print(Temperature);
		Serial.print(" / F = ");
		Serial.println(Humidity);
		 */
		// Add the same values to all logegers. They are read out at different rates!
		for(i = 0; i < NO_OF_LOGS; i++)
		{
			StatTemperature[i].AddValue(Temperature);
			StatHumidity[i].AddValue(Humidity);
		}
	}


	if ((myTime - timeLastShortTermDataStored_k1)
			> (unsigned long int)1000 * (unsigned long int)60 * TShortTermDataStoreInMin) // 1000 ms = 1s (unsigned long int)60
	{
		timeLastShortTermDataStored_k1 = myTime;
		TempQueue[LogShortTerm].add(StatTemperature[LogShortTerm].GetMeanVal(true), true);
		HumidityQueue[LogShortTerm].add(StatHumidity[LogShortTerm].GetMeanVal(true), true);
		TimeQueue[LogShortTerm].add(CurrentTimeNum, true);

		//TempQueue.PrintStatus();
	}
	if ((myTime - timeLastLongTermDataStored_k1)
			> (unsigned long int)1000 * (unsigned long int) 3600 * TLongTermDataStoreInHour) // 1000 ms = 1s (unsigned long int)60
	{
		timeLastLongTermDataStored_k1 = myTime;
		TempQueue[LogLongTerm].add(StatTemperature[LogLongTerm].GetMeanVal(true), true);
		HumidityQueue[LogLongTerm].add(StatHumidity[LogLongTerm].GetMeanVal(true), true);
		TimeQueue[LogLongTerm].add(CurrentTimeNum, true);
	}

	return (1);
}

/*==========================================================================================================================
 * Get latest temperature => use the short term log
 *
 */
float TempLogger::GetTemperature()
{
	float Value;

	if(TempQueue[LogShortTerm].getLast(&Value))
	{
		return(Value);
	}
	else
	{
		return(0.0);
	}
}

/*==========================================================================================================================
 * Get latest humidity  => use the short term log
 *
 */
float TempLogger::GetHumidity()
{
	float Value;

	if(HumidityQueue[LogShortTerm].getLast(&Value))
	{
		return(Value);
	}
	else
	{
		return(0.0);
	}
}

/*==========================================================================================================================
 * Get latest time stamp
 *
 */
long TempLogger::GetTime()
{
	return(CurrentTimeNum);
}

/*==========================================================================================================================
 * Get the error message string
 *
 */
String TempLogger::GetErrorMessage()
{
	if(ErrorStr.isEmpty())
	{
		return(String("Keine Fehlermeldungen aufgelaufen"));
	}
	else
	{
		return(ErrorStr);
	}
}


/*==========================================================================================================================
 * Reset the queues and delete all data stored in this logger
 *
 */
bool TempLogger::ResetBuffers()
{
	TempQueue[LogShortTerm].deleteMe();
	HumidityQueue[LogShortTerm].deleteMe();
	TimeQueue[LogShortTerm].deleteMe();

	TempQueue[LogLongTerm].deleteMe();
	HumidityQueue[LogLongTerm].deleteMe();
	TimeQueue[LogLongTerm].deleteMe();

	return(true);
}

/*==========================================================================================================================
 * Set the currently active log: either short term or long term
 * The pointer of the current log is returned by GetDataQueue()
 *
 */
bool TempLogger::SetCurrentLog(enCurrentLog myCurrentLog)
{
	if((myCurrentLog == LogShortTerm) || (myCurrentLog == LogLongTerm))
	{
		CurrentLog = myCurrentLog;
		return(true);
	}
	else
	{
		return(false);
	}

}

/*==========================================================================================================================
 * Return a pointer to the requested data queue to allow easy external access to the values stored in this queue
 *
 */
KBuffer <float> * TempLogger::GetDataQueue(enDataQueueType DataQueueType, enCurrentLog LogType)
{
	if((LogType == LogShortTerm) || (LogType == LogLongTerm))
	{
		if(DataQueueType == DataTempVals)
		{
			return(&TempQueue[LogType]);
		}
		if(DataQueueType == DataHumidityVals)
		{
			return(&HumidityQueue[LogType]);
		}
	}
	// we should not get here!
	return(NULL);

}


/*==========================================================================================================================
 * Return a pointer to the requested time queue to allow easy external access to the values stored in this queue
 *
 */
KBuffer <unsigned long> *  TempLogger::GetTimeQueue(enCurrentLog LogType)
{
	if((LogType == LogShortTerm) || (LogType == LogLongTerm))
	{
		return(&TimeQueue[LogType]);
	}
	// we should not get here!
	return(NULL);
}
