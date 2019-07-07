#include "WetSensor.h"
#include "PhotoResistor.h"
#include <avr/wdt.h>
#include <avr/sleep.h>

WetSensor w_sensor(A0); // Объект-датчик влажности (пин)
PhotoResistor photo_r(0, 12, A1); // Объект-датчик освещенности. (начало выделенной памяти EEPROM, конец памяти, пин)
const byte PUMP = 5; // пин помпы
const byte LAMP = 9; // пин ленты
const byte FAN = 6; // пин вентилятора

//------------------НАСТРОЙКИ-------------------
const int WET_LIMIT = 180; // лимит значения датчика влажности почвы
const int BR_LIMIT = 3; // лимит среднего значения фоторезистора
const int BR_LIMIT_LAMP = 7; // лимит освещенности до которого можно включать исскустенное освещение
const int PUMP_LIMIT = 10000; // время работы помпы
const int Q_TO_SKIP_CICLE = 450; // Количество циклов сна без какой либо деятельности.
//Если время сна в power_down mode 8 секунд то 450 пропусков = 1 час


int photoresistor_vol;
int cicle_ind = 0; // Это счетчик циклов сна
bool can = false; // разрешает выполнить действие

void setup() {

  pinMode(PUMP, OUTPUT);
  pinMode(LAMP, OUTPUT);
  pinMode(FAN, OUTPUT);
  
  analogWrite(PUMP, 0);
  digitalWrite(LAMP, LOW);
  digitalWrite(FAN, LOW);

  pinMode(13, OUTPUT);
  Serial.begin(9600);

}

void loop() {

  /*действие вынесено сюда так как до основного цикла, возможно,
    ардуино не достаточно проснулас после "сна" что бы управлять пинами
    это не точно, и получилось в следствии долгих неудачных опытов*/
  if (can) {
    action();
    can = false;
  }

  cicle_ind++;

  Serial.println("=====in_sleep_cicle(possible_to_command)=====");
  Serial.println("commands: a, g, c, p");
  delay(500);

  /*Блок с условием - для отправки команд при отладке
    команды:
    а - выполнить функцию action
    g - получить данные с датчиков
    с - обнулить все данные
    p - вывести данные в сериал*/
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
      Serial.print("BRIGHTNESS: ");
      Serial.println(photo_r.getBrightness());
      Serial.print("AVERAGE_BRIGHTNESS: ");
      Serial.println(photo_r.getAverageBrightness());
      Serial.print("WETNESS: ");
      Serial.println(w_sensor.getAverageVolOfWetness());
      Serial.print("CICLE_IND_NOW: ");
      Serial.println(cicle_ind);

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

  if (photo_r.getHeadLevel(photo_r.getBrightness()) < BR_LIMIT_LAMP) {
    digitalWrite(LAMP, HIGH);
    digitalWrite(LAMP, HIGH);
  } else {
    digitalWrite(LAMP, LOW);
    digitalWrite(LAMP, LOW);
  }

  if (cicle_ind > Q_TO_SKIP_CICLE) {

    getData();
    cicle_ind = 0;
    /*в отношении яркости меньше - ярче*/
    if (photo_r.getAverageBrightness() < BR_LIMIT &&
        photo_r.getHeadLevel(photo_r.getBrightness()) < BR_LIMIT &&
        w_sensor.getAverageVolOfWetness() > WET_LIMIT) {

      Serial.println("action_cond");
      can = true;

    }
  }

}

void action() {

  Serial.println("ACTION");
  digitalWrite(13, HIGH);

  analogWrite(PUMP, 80);
  delay(PUMP_LIMIT);
  analogWrite(PUMP, 0);

  digitalWrite(13, LOW);

  delay(100);

}

/*получает данные с датчика освещенности и записывает его в EEPROM*/
void getData() {

  photoresistor_vol = photo_r.getBrightness();

  photo_r.writeNextBrightness(photoresistor_vol);
  //photo_r.writeAverageVol();//FOR_TEST

  Serial.print("photoresistor ->");
  Serial.println(photoresistor_vol);
}
