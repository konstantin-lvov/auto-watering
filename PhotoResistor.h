/**/

#ifndef PhotoResistor_h
#define PhotoResistor_h

#include <Arduino.h>
#include <EEPROM.h>

class PhotoResistor {
	public:
		PhotoResistor(byte begin, byte end, byte pin);
		byte getAverageBrightness();
		void writeNextBrightness(int vol);
		int getHeadLevel(int x);
		void writeAverageVol();//FOR_TEST
		void printEEPROM();
		int getBrightness();
    void clearAddress();
    
	private:
		byte pin;
		byte space_begin;
		byte space_end;
		byte hours_limit;
		byte address;
    byte FT_address;//FOR_TEST
};

#endif
