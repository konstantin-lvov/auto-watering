#include "WetSensor.h" /*pinMode, digitalWrite on a board*/
#include "PhotoResistor.h"
#include <avr/wdt.h>
#include <avr/sleep.h>


WetSensor w_sensor(A0);/*> 1000 (very dry)
                         ... not enough information about that sensor,
                         perhaps < 500 this is quite wet soil */
const int WET_LIMIT = 600;

PhotoResistor photo_r(0, 12, A1);
const int BR_LIMIT = 6;

const byte PUMP = 5;
const int PUMP_LIMIT = 10000;

int photoresistor_vol;
int wet_sensor_vol;
int cicle_ind = 0;

void setup() {

  pinMode(PUMP, OUTPUT);
  digitalWrite(PUMP, LOW);

  Serial.begin(9600);
  Serial.println("SETUP");

}

void loop() {
  EEPROM.get(40, cicle_ind);
  cicle_ind++;
  EEPROM.put(40, cicle_ind);


  Serial.println("=====in_sleep_cicle(possible_to_command)=====");
  delay(500);

  if (Serial.available()) {

    String tmp;

    while (Serial.available()) tmp += (char)Serial.read();

    if (tmp.substring(0, 1).equals("a") || tmp.substring(0, 1).equals("c") ||
        tmp.substring(0, 1).equals("p") || tmp.substring(0, 1).equals("g")) {
      tmp = tmp.substring(0, 1);
      Serial.println("substring -------" + tmp);
    }

    if (tmp.equals("a")) {
      action();
    }
    if (tmp.equals("g")) {
      Serial.println(photo_r.getBrightness());
      Serial.println(w_sensor.getAverageVolOfWetness());
    }
    if (tmp.equals("c")) {
      photo_r.clearAddress();
      getData();
      photo_r.printEEPROM();
    }
    if (tmp.equals("p")) {
      photo_r.printEEPROM();
    } else {
      int read_ee;
      EEPROM.get(tmp.toInt(), read_ee);
      Serial.println("E_PROM " + tmp + " " + String(read_ee));
    }
    delay(10);
  }

  delay(100);

  wdt_enable(WDTO_8S); //Задаем интервал сторожевого таймера (8с)
  WDTCSR |= (1 << WDIE); //Устанавливаем бит WDIE регистра WDTCSR для
  //разрешения прерываний от сторожевого таймера
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //Устанавливаем интересующий нас режим
  sleep_mode(); // Переводим МК в спящий режим

}
/* После его выполнения продолжится работа основной программы,
  т.е. выполнится следующая после sleep_mode() команда.*/
ISR (WDT_vect) {
  wdt_disable();

  if (cicle_ind > 450) {
    EEPROM.put(40, (int)0);
    getData();

    if (photo_r.getAverageBrightness() > BR_LIMIT &&
        wet_sensor_vol > WET_LIMIT) {

      action();
    }
  }

}

void action() {
  Serial.println("ACTION");

  digitalWrite(PUMP, HIGH);
  delay(PUMP_LIMIT);
  digitalWrite(PUMP, LOW);
}

void getData() {

  photoresistor_vol = photo_r.getBrightness();
  wet_sensor_vol = w_sensor.getAverageVolOfWetness();

  photo_r.writeNextBrightness(photoresistor_vol);
  photo_r.writeAverageVol();

  Serial.print("photoresistor ->");
  Serial.println(photoresistor_vol);
  Serial.print("wet_sensor ->");
  Serial.println(wet_sensor_vol);
}
