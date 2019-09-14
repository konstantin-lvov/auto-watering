#include "WetSensor.h"
#include <EEPROM.h>

unsigned long unsigned_long_size = 4294967295;

/*В еепром записываем:
  current_hour - 0 addr
  current_minutes - 1 addr
  last_watering_hour - 2 addr
  GOOD_NIGHT - 3 addr
  GOOD_MORNING - 4 addr
  WET_LIMIT - 5 addr
*/
byte base_hour = 12;
byte base_minutes = 0;
byte current_hour = 12;
byte current_minutes = 0;
byte last_watering_hour;
byte GOOD_NIGHT = 22;
byte GOOD_MORNING = 10;
byte watering_pass_time;
byte fan_speed = 0;
unsigned long base_millis = 0;
unsigned long current_millis = 0;
unsigned long passed_millis = 0;
unsigned long passed_minutes;
int wetness;

WetSensor w_sensor(A0); // Объект-датчик влажности (пин)
const byte BUTTON = 7;
const byte PUMP = 5; // пин помпы
const byte LAMP = 9; // пин ленты
const byte FAN = 3; // пин вентилятора

//------------------НАСТРОЙКИ-------------------
int WET_LIMIT = 160; // лимит значения датчика влажности почвы
const int PUMP_LIMIT = 10000; // время работы помпы
const byte WATERING_INTERVAL = 2;

void setup() {

  current_hour        = EEPROM.read(0);
  current_minutes     = EEPROM.read(1);
  last_watering_hour  = EEPROM.read(2);
  GOOD_NIGHT          = EEPROM.read(3);
  GOOD_MORNING        = EEPROM.read(4);
  WET_LIMIT           = EEPROM.read(5);
  fan_speed           = EEPROM.read(6);

  base_hour = current_hour;
  base_minutes = current_minutes;

  base_millis = millis();


  pinMode(BUTTON, INPUT);
  pinMode(PUMP, OUTPUT);
  pinMode(LAMP, OUTPUT);
  pinMode(FAN, OUTPUT);

  digitalWrite(BUTTON, HIGH);
  analogWrite(PUMP, 0);
  analogWrite(FAN, fan_speed);
  digitalWrite(LAMP, LOW);
  //digitalWrite(FAN, LOW);

  pinMode(13, OUTPUT);
  Serial.begin(9600);

}

void loop() {
  
  command_line();
  delay(50);
  button_checkout();

  if (millis() - current_millis > 5000) {
    Serial.println("##########################");
    
    wetness = w_sensor.getAverageVolOfWetness();

    print_current_info();

    printTime();
    
    checkConditions();
    
    Serial.println("##########################");
  }


}

void print_current_info(){
  if (current_hour > GOOD_MORNING && current_hour < GOOD_NIGHT) {
      Serial.println("===Day===");
    } else {
      Serial.println("===Night===");
    }

    Serial.println(String("ground wetness - ") + wetness);
    Serial.print("current time - ");
}

void button_checkout() {

  if (digitalRead(BUTTON) == LOW) { //если кнопка нажата
    delay(300);

    if (digitalRead(BUTTON) == LOW) { //если кнопка все еще нажата то изменяем скорость вентилятора
      bool slowdown = false;
      while (digitalRead(BUTTON) == LOW) {
        if (slowdown == false) {
          fan_speed++;
          delay(10);
          if (fan_speed >= 253) {
            slowdown = true;
          }
        }
        if (slowdown == true) {
          fan_speed--;
          delay(10);
          if (fan_speed <= 2) {
            slowdown = false;
          }
        }
        analogWrite(FAN, fan_speed);
        Serial.println(String("Fan speed: ") + fan_speed);
      }
    } else { // иначе запомнить все настройки
      digitalWrite(13, HIGH);
      delay(500);
      digitalWrite(13, LOW);
      EEPROM.update(0, current_hour);
      EEPROM.update(1, current_minutes);
      EEPROM.update(2, last_watering_hour);
      EEPROM.update(3, GOOD_NIGHT);
      EEPROM.update(4, GOOD_MORNING);
      EEPROM.update(5, WET_LIMIT);
      EEPROM.update(6, fan_speed);

      Serial.println(String("current_hour ") + EEPROM.read(0));
      Serial.println(String("current_minutes ") + EEPROM.read(1));
      Serial.println(String("last_watering_hour ") + EEPROM.read(2));
      Serial.println(String("GOOD_NIGHT ") + EEPROM.read(3));
      Serial.println(String("GOOD_MORNING ") + EEPROM.read(4));
      Serial.println(String("WET_LIMIT ") + EEPROM.read(5));
      Serial.println(String("fan_speed ") + EEPROM.read(6));
    }

  }
}

void checkConditions() {

  if (current_hour >= GOOD_MORNING && current_hour <= GOOD_NIGHT) {
    digitalWrite(LAMP, HIGH);
    analogWrite(FAN, fan_speed);
  } else {
    digitalWrite(LAMP, LOW);
    analogWrite(FAN, 0);
  }

  //wpt=20 ct=9 9-20=-11
  //если полив осуществлялся более 12 часов назад то нужно проверять на отрицательное значение
  watering_pass_time =  current_hour - last_watering_hour;
  if (watering_pass_time < 0) {
    watering_pass_time *= -1;
  }

  if ( watering_pass_time >= WATERING_INTERVAL
       && current_hour > GOOD_MORNING && current_hour < GOOD_NIGHT
       && wetness > WET_LIMIT) {
    action();
    last_watering_hour = current_hour;
  }
}

void action() {

  Serial.println("ACTION");
  digitalWrite(13, HIGH);

  analogWrite(PUMP, 80);
  delay(PUMP_LIMIT);
  analogWrite(PUMP, 0);

  digitalWrite(13, LOW);

}

void printTime() {

  current_millis = millis();

  //определение момента когда обнулится  millis()
  if (current_millis - base_millis < 0) {
    passed_millis = unsigned_long_size - base_hour + current_millis;//dich polnaya nado peredelat
  } else {
    passed_millis = current_millis - base_millis;
  }

  passed_minutes = ((passed_millis / 1000) / 60);

  if (base_minutes + passed_minutes > 59) {
    current_minutes = base_minutes + passed_minutes - 60;
    current_hour = base_hour + 1;
    base_minutes = current_minutes;
    base_hour = current_hour;
    if (base_hour > 23) {
      base_hour = 0;
      current_hour = 0;
    }
    base_millis = millis();
  } else {
    current_minutes = base_minutes + passed_minutes;
  }

  if (base_hour < 10) {
    Serial.print(String("0") + current_hour);
  } else {
    Serial.print(current_hour);
  }

  Serial.print(":");

  if (current_minutes < 10) {
    Serial.println(String("0") + current_minutes);
  } else {
    Serial.println(current_minutes);
  }
}



void command_line() {

  if (Serial.available()) {

    String tmp;
    String f_letter;
    String end_part;
    String tmp_h;
    String tmp_m;

    while (Serial.available()) tmp += (char)Serial.read();

    Serial.println(String("serial read : ") + tmp);

    for (byte i = 0; i < tmp.length(); i++) {

      if ((int)tmp.charAt(i) != 32) {
        f_letter += tmp.charAt(i);
      } else {
        end_part = tmp.substring(i, tmp.length());
        break;
      }
    }

    f_letter.trim();
    end_part.trim();

    Serial.println(String("accepted sequense : ") + f_letter + end_part);

    if (f_letter.equals("sw")) {
      WET_LIMIT = end_part.toInt();
      Serial.print("WET_LIMIT - ");
      Serial.println(WET_LIMIT);
      delay(5000);
    }

    if (f_letter.equals("h")) {
      Serial.println("--------------------");
      Serial.println("-->possible command:");
      Serial.println("a (action)");
      Serial.println("t hh:mm (set current time)");
      Serial.println("sm hh (set GOOD_MORNING)");
      Serial.println("sn hh (set GOOD_NIGHT)");
      Serial.println("sw int (set WET_LIMIT)");
      Serial.println("--------------------");
      delay(5000);
    }

    if (f_letter.equals("sm")) {
      GOOD_MORNING = end_part.toInt();
      Serial.print("MORNING - ");
      Serial.println(GOOD_MORNING);
      Serial.print("NIGHT - ");
      Serial.println(GOOD_NIGHT);
      delay(5000);
    }

    if (f_letter.equals("sn")) {
      GOOD_NIGHT = end_part.toInt();
      Serial.print("MORNING - ");
      Serial.println(GOOD_MORNING);
      Serial.print("NIGHT - ");
      Serial.println(GOOD_NIGHT);
      delay(5000);
    }


    if (f_letter.equals("t")) {
      byte colon_ind = 0;
      for (byte i = 0; i < end_part.length(); i++) {
        colon_ind = i; // сюда запишется индекс двоеточия когда сработает оператор break
        if (end_part.charAt(i) != ':') {
          tmp_h += end_part.charAt(i);
        } else {
          break;
        }
      }
      //f_letter = t, end_part = 20:13
      // |20:13
      // |01234
      for (byte i = colon_ind + 1; i < end_part.length(); i++) {
        if ((int)end_part.charAt(i) != 10) {
          tmp_m += end_part.charAt(i);
        } else {
          break;
        }
      }

      base_hour = tmp_h.toInt();
      base_minutes = tmp_m.toInt();
      current_hour = base_hour;
      last_watering_hour = base_hour;
      base_millis = millis();
    }

    if (f_letter.equals("a")) {
      action();
    }
  }
}
