
/*
this class allows to work with sensor of wetness
consisting of two strips of steel
*/

#ifndef WetSensor_h
#define WetSensor_h

#include "Arduino.h"

class WetSensor {
	public:
		WetSensor(int pin);
		int getAverageVolOfWetness();
		int getVolOfWetness();
		float translateInVoltage();
	private:
		int currentVol;
		int analogPin;
		int sensorArray [10];
		int average;
};

#endif