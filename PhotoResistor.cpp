#include "Arduino.h"
#include "PhotoResistor.h"

PhotoResistor::PhotoResistor(byte begin_, byte end_, byte pin) {//0,12,a1
  Serial.println("CONSTRUCTOR");
  PhotoResistor::pin = pin;
  PhotoResistor::space_begin = begin_;
  PhotoResistor::space_end = end_;
  PhotoResistor::hours_limit = end_;

  pinMode(pin, INPUT);

  /*FOR_TEST*/
  if (EEPROM.read(13) != 255)
    PhotoResistor::FT_address = EEPROM.read(13);
  else
    PhotoResistor::FT_address = 14;

  if (EEPROM.read(space_begin) != 255)
    PhotoResistor::address = EEPROM.read(space_begin);
  else
    PhotoResistor::address = space_begin + 1;
}

/*just in case anti-noise*/
int PhotoResistor::getBrightness() {

  int average_b = 0;
  for (byte i = 0; i < 10; i++) {
    average_b += analogRead(PhotoResistor::pin);
    delay(100);
  }
  average_b /= 10;
  return average_b;
}



byte PhotoResistor::getAverageBrightness() {

  byte average = 0;
  byte ind = space_begin + 1;
  do {
    average += EEPROM.read(ind);
    ind++;
  } while (EEPROM.read(ind) > 0 && ind != space_end);
  average /= ind - 1;
  Serial.println("AVERAGE " + String(average) + " IND " + String(ind));
  return average;

}


void PhotoResistor::writeNextBrightness(int vol) {

  byte head_l_v = getHeadLevel(vol);
  EEPROM.update(address, head_l_v);
  Serial.println("Шаг: " + String(EEPROM.read(0)));
  Serial.println("write: " + String(head_l_v) + " on address: " + String(address));
  address++;
  if (address == hours_limit + 1)
    address = space_begin + 1;
  EEPROM.write(space_begin, address);
}


int PhotoResistor::getHeadLevel(int x) {

  byte tmp = 0;
  while (x > 10) {
    x /= 10;
    tmp++;
  }
  if (tmp == 1)
    return 1;
  if (tmp == 3)
    return 10;
  else
    return x;

}

/*FOR_TEST*/
void PhotoResistor::writeAverageVol() {

  byte tmp = 0;
  tmp = getAverageBrightness();
  EEPROM.update(FT_address, tmp);
  Serial.println("Шаг_FT: " + String(EEPROM.read(13)));
  FT_address++;
  if (FT_address == 38)
    FT_address = 14;
  EEPROM.write(13, FT_address);

}

void PhotoResistor::printEEPROM() {

  Serial.println("printeeprom");
  int ind = 0;
  while (EEPROM.read(ind + 1) != 255) {
    Serial.print(String(ind) + ": ");
    Serial.println(EEPROM.read(ind));
    if (ind == 0 || ind == 12 || ind == 13) {
      Serial.println("=====================================================");
    }
    ind++;
  }

}

void PhotoResistor::clearAddress() {

  for (byte i = 0; i < 38; i++) {
    EEPROM.update(i, 0);
  }
  Serial.println("addresses CLEAR");
  PhotoResistor::address = 1;
  PhotoResistor::FT_address = 14;
  EEPROM.write(0, address);
  EEPROM.write(13, FT_address);
  EEPROM.put(40, (int)0);

}
