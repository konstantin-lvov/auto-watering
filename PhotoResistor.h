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
		void printEEPROM();
		int getBrightness();
    void clearAddress();
    
	private:
		byte pin;
		byte space_begin;
		byte space_end;
		byte hours_limit;// Количество часов для мониторинга
		byte address;// Указатель на текущую ячейку памяти для записи уровня освещенности
};

#endif
