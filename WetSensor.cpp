#include "Arduino.h"
#include "WetSensor.h"

WetSensor::WetSensor(int pin) {
  WetSensor::analogPin = pin;
  pinMode(analogPin, INPUT);
  digitalWrite(analogPin, HIGH);
}

int WetSensor::getAverageVolOfWetness() {

  average = 0;

  for (int i = 0 ; i < 10; i++) {
    average += analogRead(analogPin);
    delay(100);
  }
  average = average / 10;

  return average;

}

int WetSensor::getVolOfWetness() {
  currentVol = analogRead(analogPin);

  return currentVol;
}

float WetSensor::translateInVoltage() {
  return (float)analogRead(analogPin) * 5. / 1024.;
}
