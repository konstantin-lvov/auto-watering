#include "WetSensor.h"
#include <EEPROM.h>

//=======================================ПЕРЕМЕННЫЕ=======================================
unsigned long unsigned_long_size = 4294967295; // переменная для хранения максимального значения помещаемого в unsigned long
byte base_hour = 12; 
byte base_minutes = 0;
byte current_hour = 12;
byte current_minutes = 0;
byte last_watering_hour;

byte watering_pass_time;
byte fan_speed = 0;
unsigned long base_millis = 0;
unsigned long current_millis = 0;
unsigned long passed_millis = 0;
unsigned long passed_minutes;
unsigned long last_print_millis;
int wetness;
bool slowdown = false;

//=======================================ПИНЫ===========================================
WetSensor w_sensor(A0); // Объект-датчик влажности (пин)
const byte BUTTON = 7;
const byte PUMP = 5; // пин помпы
const byte LAMP = 9; // пин ленты
const byte FAN = 3; // пин вентилятора

//=======================================НАСТРОЙКИ=======================================
/* WET_LIMIT - лимит влажности почвы. То значение при котором будем считать что пора полить растение. 
 * Проще всего установив физический датчик в почву, сразу полить растение.
 * После этого в serial можно увидеть значение влажной почвы. Прибавив к этому значениею 50-100 едениц
 * (в зависимости от потребностей растения) получим значение которое нужно подставить в данную переменную.
 */
int WET_LIMIT = 160;

/*  PUMP_LIMIT - время работы помпы. Нужно учитывать длинну трубок от помпы до растения. Что бы не случилось
 *  того, что помпа отработала, а вода до растения еще не добралась. Указывается в миллисекундах.
 */
int PUMP_LIMIT = 10000;

/*  WATERING_INTERVAL - минимальный интервал, который будет выдержан перед повторным поливом растения. При установленном значении
 *  равно 0 - полив будет возможен сразу после предыдущего полива. Если ипользуются датчики влажности, можно смело отключать
 *  данныую фунцкию, т.к. сразу после полива значение влажности почвы будет ниже WET_LIMIT и это не даст поливу сработать вновь.
 */
byte WATERING_INTERVAL = 0;
/*  GOOD_MORNING и GOOD_NIGHT  - время (в часах), которое используется для определения времени для включения освещения, а так же
 *   возможности для включения помпы (т.е. полива растения). Освещение будет включено и помпа сможет включиться если время отсчитываемое
 *   программно будет больше GOOD_MORNING и меньше GOOD_NIGHT
 */
byte GOOD_MORNING = 10;
byte GOOD_NIGHT = 22;


void setup() {

  current_hour        = EEPROM.read(0);
  current_minutes     = EEPROM.read(1);
  last_watering_hour  = EEPROM.read(2);
  GOOD_NIGHT          = EEPROM.read(3);
  GOOD_MORNING        = EEPROM.read(4);
  WET_LIMIT           = EEPROM.read(5);
  fan_speed           = EEPROM.read(6);
  WATERING_INTERVAL   = EEPROM.read(7);
  PUMP_LIMIT          = EEPROM.read(8);

  base_hour = current_hour;
  base_minutes = current_minutes;

  base_millis = millis();
  last_print_millis = base_millis;


  pinMode(BUTTON, INPUT);
  pinMode(PUMP, OUTPUT);
  pinMode(LAMP, OUTPUT);
  pinMode(FAN, OUTPUT);

  digitalWrite(BUTTON, HIGH);
  analogWrite(PUMP, 0);
  analogWrite(FAN, fan_speed);
  digitalWrite(LAMP, LOW);

  pinMode(13, OUTPUT);
  Serial.begin(9600);

}

void loop() {
/*
1. Собираем данные с датчика влажности (внутри реализовано с delay на 100 миллисекунд)
*/
  wetness = w_sensor.getAverageVolOfWetness();
/*
2. Устанавливаем текущее время. Происходит не чаще чем раз в 60-70 миллисекунд.
*/
  set_clock();
/*
3. Проверяем нет ли команд в serial для исполнения. Без задержки работало не корректно.
*/  
  command_line();
  delay(50);
/*
4. Проверяем не нажата ли кнопка управляющая скоростью вентилятора и выводом хранимых переменных в serial
*/   
  button_checkout();
/*
5. Проверяем условия для полива и освещения. И поливаем если условия совпадли.
*/  
  checkConditions();
/*
6. Печатаем время и текущее состояние влажности в serial каждые 5 сек.
*/
  if (millis() - last_print_millis > 5000) {
    
    print_current_info();

  }


}

void print_current_info(){

  last_print_millis = millis();
  
  Serial.println("##########################");
  
  if (current_hour > GOOD_MORNING && current_hour < GOOD_NIGHT) {
      Serial.println("===Day===");
    } else {
      Serial.println("===Night===");
    }

    Serial.println(String("ground wetness - ") + wetness);
    
    Serial.print("current time - ");

  if (current_hour < 10) {
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
    Serial.println("##########################");
}

void button_checkout() {

  if (digitalRead(BUTTON) == LOW) { //если кнопка нажата
    delay(300);

    if (digitalRead(BUTTON) == LOW) { //если кнопка все еще нажата то изменяем скорость вентилятора
      
      while (digitalRead(BUTTON) == LOW) {
        if (slowdown == false) {
          fan_speed++;
          delay(10);
          if (fan_speed >= 255) {
            slowdown = true;
            delay(500);
          }
        }
        if (slowdown == true) {
          fan_speed--;
          delay(10);
          if (fan_speed < 10){
            delay(300);
          }
          if (fan_speed <= 0) {
            slowdown = false;
            delay(500);
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
      EEPROM.update(7, WATERING_INTERVAL);
      EEPROM.update(8, PUMP_LIMIT);

      Serial.println(String("current_hour ") + EEPROM.read(0));
      Serial.println(String("current_minutes ") + EEPROM.read(1));
      Serial.println(String("last_watering_hour ") + EEPROM.read(2));
      Serial.println(String("GOOD_NIGHT ") + EEPROM.read(3));
      Serial.println(String("GOOD_MORNING ") + EEPROM.read(4));
      Serial.println(String("WET_LIMIT ") + EEPROM.read(5));
      Serial.println(String("fan_speed ") + EEPROM.read(6));
      Serial.println(String("WATERING_INTERVAL ") + EEPROM.read(7));
      Serial.println(String("PUMP_LIMIT ") + EEPROM.read(8));
    }
    EEPROM.update(6, fan_speed);
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

void set_clock(){
  
   current_millis = millis();

  //определение момента когда обнулится  millis()
  if (current_millis - base_millis < 0) {
    passed_millis = unsigned_long_size - base_millis + current_millis;
  } else {
    passed_millis = current_millis - base_millis; // количество милисекунд прошедших с момента смены часа
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
}

void command_line() {

  if (Serial.available()) {

    String tmp;
    String f_letter;
    String end_part;
    String tmp_h;
    String tmp_m;

    while (Serial.available()) tmp += (char)Serial.read();

    //Serial.println(String("serial read : ") + tmp);

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

    //Serial.println(String("accepted sequense : ") + f_letter + end_part);

    if (f_letter.equals("h")) {
      Serial.println("--------------------");
      Serial.println("-->possible command:");
      Serial.println("a (action)");
      Serial.println("t hh:mm (set current time)");
      Serial.println("sgom hh (set GOOD_MORNING)");
      Serial.println("sgon hh (set GOOD_NIGHT)");
      Serial.println("swel int (set WET_LIMIT)");
      Serial.println("swai hh (set WATERING_INTERVAL)");
      Serial.println("spul millis (set PUMP_LIMIT)");
      Serial.println("--------------------");
      delay(5000);
    }


if (f_letter.equals("spul")) {
      Serial.print("old PUMP_LIMIT - ");
      Serial.println(PUMP_LIMIT);
      PUMP_LIMIT = end_part.toInt();
      Serial.print("new PUMP_LIMIT - ");
      Serial.println(PUMP_LIMIT);
      EEPROM.update(8, PUMP_LIMIT);
      delay(5000);
    }
    
    if (f_letter.equals("swel")) {
      Serial.print("old WET_LIMIT - ");
      Serial.println(WET_LIMIT);
      WET_LIMIT = end_part.toInt();
      Serial.print("new WET_LIMIT - ");
      Serial.println(WET_LIMIT);
      EEPROM.update(5, WET_LIMIT);
      delay(5000);
    }

    if (f_letter.equals("swai")) {
      Serial.print("old WATERING_INTERVAL - ");
      Serial.println(WATERING_INTERVAL);
      WATERING_INTERVAL = end_part.toInt();
      Serial.print("new WATERING_INTERVAL - ");
      Serial.println(WATERING_INTERVAL);
      EEPROM.update(7, WATERING_INTERVAL);
      delay(5000);
    }

    if (f_letter.equals("sgom")) {
      Serial.print("old MORNING - ");
      Serial.println(GOOD_MORNING);
      GOOD_MORNING = end_part.toInt();
      Serial.print("new MORNING - ");
      Serial.println(GOOD_MORNING);
      EEPROM.update(4, GOOD_MORNING);
      delay(5000);
    }

    if (f_letter.equals("sgon")) {
      Serial.print("old NIGHT - ");
      Serial.println(GOOD_NIGHT);
      GOOD_NIGHT = end_part.toInt();
      Serial.print("new NIGHT - ");
      Serial.println(GOOD_NIGHT);
      EEPROM.update(3, GOOD_NIGHT);
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

      Serial.println(String("Время установлено на ") + current_hour + String(":") + base_minutes);

      EEPROM.update(0, current_hour);
      EEPROM.update(1, base_minutes);
      EEPROM.update(2, last_watering_hour);
    }
    

    if (f_letter.equals("a")) {
      action();
    }
  }
}
