//#include <SoftwareSerial.h>
//const int TxD = 3;
//const int RxD = 2;

#include <Wire.h>
#include "DS3231.h"

const int SDAr = 20;
const int SCLr = 21;
DS3231 rtc(SDAr, SCLr);


#include <LiquidCrystal.h>
int Contrast=70;

const int RS = 37;
const int enable = 39;
const int d4 = 41;
const int d5 = 43;
const int d6 = 45;
const int d7 = 47;

LiquidCrystal lcd(RS, enable, d4, d5, d6, d7);

//Piezo Buzzer
const int Piezo = 31;

//LED

const int LED = 33;

//HC-SR05 x 7
const int trigMo = 13;
const int echoMo = 12;

const int trigTu = 11;
const int echoTu = 10;

const int trigWe = 9;
const int echoWe = 8;

const int trigTh = 7;
const int echoTh = 6;

const int trigFr = 5;
const int echoFr = 4;

const int trigSa = 3;
const int echoSa = 2;

const int trigSu = 24;
const int echoSu = 22;

String onDay[7] = {"Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"};
String atHour[7] = {"18:00:00", "12:00:00", "0", "14:45:15", "13:00:40", "0", "0"};
String prescription[7]= {"1 Paracetamol", "1 nurofen", "", "1 ibusinus", "1 paracetamol", "", ""};

unsigned long Time;
int duration;
float distance;

void wait(int ms)
{
  Time = millis();
  while (millis() - Time <= ms)
  {
    continue;
  }
  return;
}

void soundAlarm(int x){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("To take");
  lcd.setCursor(0,1);
  lcd.print(prescription[x]);
  digitalWrite(LED, HIGH);
  tone(Piezo, 1000); // Send 1KHz sound signal...
  wait(1000);        // ...for 1 sec
  digitalWrite(LED, LOW);
  noTone(Piezo);     // Stop sound...
  wait(1000);        // ...for 1sec
}

int taken(const int trigPin, const int echoPin){
  
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance= duration*0.034/2;
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);

  if (distance>135){
    return 1;
  }
  else return 0;
}

void setup(){
  Serial.begin(9600);

  pinMode(trigMo, OUTPUT);
  pinMode(echoMo, INPUT);
  
  pinMode(trigTu, OUTPUT);
  pinMode(echoTu, INPUT);
  
  pinMode(trigWe, OUTPUT);
  pinMode(echoWe, INPUT);
  
  pinMode(trigTh, OUTPUT);
  pinMode(echoTh, INPUT);
  
  pinMode(trigFr, OUTPUT);
  pinMode(echoFr, INPUT);
  
  pinMode(trigSa, OUTPUT);
  pinMode(echoSa, INPUT);
  
  pinMode(trigSu, OUTPUT);
  pinMode(echoSu, INPUT);
  
  pinMode(Piezo, OUTPUT);
  pinMode(LED, OUTPUT);

  rtc.begin();
  
  // The following lines can be uncommented to set the date and time
  //rtc.setDOW(FRIDAY);     // Set Day-of-Week to SUNDAY
  //rtc.setTime(12, 12, 0);     // Set the time to 12:00:00 (24hr format)
  //rtc.setDate(21, 2, 2020);   // Set the date to January 1st, 2014


//  bluetoothSerial.begin(9600);
  analogWrite(35,Contrast);
  lcd.begin(16, 2);
}

void loop(){

  // Send Day-of-Week
  Serial.print(rtc.getDOWStr());
  Serial.print(" ");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(rtc.getDOWStr());
  wait(100);
  lcd.setCursor(0, 1);
  lcd.print(rtc.getDateStr());
  wait(100);
  
  // Send date
  Serial.print(rtc.getDateStr());
  Serial.print(" -- ");

  // Send time
  Serial.println(rtc.getTimeStr());

  wait(1000);
  if(rtc.getDOWStr()=="Monday"){ 
      Serial.println("E LUNI"); 
      if((String)rtc.getTimeStr()==atHour[0] && atHour[0]!="0"){
        while(taken(trigMo, echoMo)==0){
          soundAlarm(0);
        }
      }
    }
  if(rtc.getDOWStr()=="Tuesday"){
      if((String)rtc.getTimeStr()==atHour[1] && atHour[1]!="0"){
        while(taken(trigTh, echoTh)==0){
          soundAlarm(1);
        }
      }
    }
  if(rtc.getDOWStr()=="Wednesday"){
      if((String)rtc.getTimeStr()==atHour[2] && atHour[2]!="0"){
        while(taken(trigWe, echoWe)==0){
          soundAlarm(2);
        }
      }
    }
  if(rtc.getDOWStr()=="Thursday"){
      if((String)rtc.getTimeStr()==atHour[3] && atHour[3]!="0"){
        while(taken(trigTu, echoTu)==0){
          soundAlarm(3);
        }
      }
    }
  if(rtc.getDOWStr()=="Friday"){
      Serial.println("E VINERI");
      if((String)rtc.getTimeStr()==atHour[4] && atHour[4]!="0"){
        Serial.println("E ORAAA");
        while(taken(trigFr, echoFr)==0){
          soundAlarm(4);
        }
      }
    }
  if(rtc.getDOWStr()=="Saturday"){
      if((String)rtc.getTimeStr()==atHour[5] && atHour[5]!="0"){
        while(taken(trigSa, echoSa)==0){
          soundAlarm(5);
        }
      }
    }
  if(rtc.getDOWStr()=="Sunday"){
      if((String)rtc.getTimeStr()==atHour[6] && atHour[6]!="0"){
        while(taken(trigSu, echoSu)==0){
          soundAlarm(6);
        }
      }
    }
  digitalWrite(LED, LOW);

}
