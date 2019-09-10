#include "WetSensor.h"

unsigned long unsigned_long_size = 4294967295;

byte base_hour = 12;
byte base_minutes = 0;
byte current_hour = 12;
byte current_minutes = 0;
byte last_watering_hour;
byte GOOD_NIGHT = 22;
byte GOOD_MORNING = 10;
unsigned long base_millis = 0;
unsigned long current_millis = 0;
unsigned long passed_millis = 0;
unsigned long passed_minutes;
int wetness;


WetSensor w_sensor(A0); // Объект-датчик влажности (пин)
const byte PUMP = 5; // пин помпы
const byte LAMP = 9; // пин ленты
const byte FAN = 3; // пин вентилятора

//------------------НАСТРОЙКИ-------------------
int WET_LIMIT = 160; // лимит значения датчика влажности почвы
const int PUMP_LIMIT = 10000; // время работы помпы
const byte WATERING_INTERVAL = 2;

void setup() {
  last_watering_hour = current_hour;
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
  wetness = w_sensor.getAverageVolOfWetness();
  
  Serial.println("##########################");
  if(current_hour > GOOD_MORNING && current_hour < GOOD_NIGHT){
    Serial.println("===Day mode===");
  } else {
    Serial.println("===Night mode===");
  }
  
  Serial.println(String("ground wetness - ") + wetness);
  Serial.print("current time - ");
  
  printTime();
  command_line();
  checkConditions();
  Serial.println("##########################");
  
}

void checkConditions(){

  if(current_hour >= GOOD_MORNING && current_hour <= GOOD_NIGHT){
    digitalWrite(LAMP, HIGH);
    digitalWrite(FAN, HIGH);
  } else {
    digitalWrite(LAMP, LOW);
    digitalWrite(FAN, LOW);
  }

  if(current_hour - last_watering_hour >= WATERING_INTERVAL
      && current_hour > GOOD_MORNING && current_hour < GOOD_NIGHT
      && wetness > WET_LIMIT){
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

  delay(100);

}

void printTime(){

  current_millis = millis();

  //определение момента когда обнулится  millis()
  if(current_millis - base_millis < 0){
   passed_millis = unsigned_long_size - base_hour + current_millis;
  } else {
    passed_millis = current_millis - base_millis;
  }
  
  passed_minutes = ((passed_millis / 1000) / 60);

  if(base_minutes + passed_minutes > 59){
    current_minutes = base_minutes + passed_minutes - 60;
    current_hour = base_hour + 1;
    base_minutes = current_minutes;
    base_hour = current_hour;
    if(base_hour > 23){
      base_hour = 0;
      current_hour = 0;
    }
    base_millis = millis();
  } else {
    current_minutes = base_minutes + passed_minutes;
  }

  if(base_hour < 10){
    Serial.print(String("0") + current_hour);
  } else {
    Serial.print(current_hour);
  }

  Serial.print(":");

  if(current_minutes < 10){
    Serial.println(String("0") + current_minutes);
  } else {
    Serial.println(current_minutes);
  }
}



void command_line(){

  if (Serial.available()) {

    String tmp;
    String f_letter;
    String end_part;
    String tmp_h;
    String tmp_m;

    while (Serial.available()) tmp += (char)Serial.read();

    for(byte i = 0; i < tmp.length(); i++){
      
      if((int)tmp.charAt(i) != 32){
        f_letter += tmp.charAt(i);
      } else {
        end_part = tmp.substring(i, tmp.length());
        break;
      }
    }

    f_letter.trim();
    end_part.trim();

    Serial.println(String("accepted sequense : ") + f_letter + end_part);

    /*if (tmp.substring(0, 1).equals("a") || tmp.substring(0, 1).equals("t")) {
      f_letter = tmp.substring(0, 1);
      end_part = tmp.substring(1, tmp.length());
      Serial.println(String("accepted sequense : ") + f_letter + end_part);
    }*/

    if(f_letter.equals("sw")){
      WET_LIMIT = end_part.toInt();
      Serial.print("WET_LIMIT - ");
      Serial.println(WET_LIMIT);
    }
    
    if(f_letter.equals("h")){
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

    if (f_letter.equals("sm")){
      GOOD_MORNING = end_part.toInt();
      Serial.print("MORNING - ");
      Serial.println(GOOD_MORNING);
      Serial.print("NIGHT - ");
      Serial.println(GOOD_NIGHT);
      delay(1000);
    }

    if (f_letter.equals("sn")){
      GOOD_NIGHT = end_part.toInt();
      Serial.print("MORNING - ");
      Serial.println(GOOD_MORNING);
      Serial.print("NIGHT - ");
      Serial.println(GOOD_NIGHT);
      delay(1000);
    }


    if (f_letter.equals("t")){
      byte colon_ind = 0;
      for (byte i = 0; i < end_part.length(); i++){
        colon_ind = i; // сюда запишется индекс двоеточия когда сработает оператор break
        if(end_part.charAt(i) != ':'){
          tmp_h += end_part.charAt(i);
        } else {
          break;
        }
      }
      //f_letter = t, end_part = 20:13
      // |20:13
      // |01234
      for (byte i = colon_ind + 1; i < end_part.length(); i++){
        if((int)end_part.charAt(i) != 10){
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
