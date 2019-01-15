#include <ESP32Servo.h>
#include "servo.h"

Servo servo;
const int servoPin = 32;
const int servoDown = 110;
const int servoUp = 20;

int servoPos = 0; //in percent - 0 is down

void servoSetup(){
  servo.attach(servoPin);
  moveServo(UP);
}


void moveServo(int percent){
  percent = constrain(percent, 0, 100);
  servoPos = percent;
  
  int pos = map(percent, 0, 100, servoDown, servoUp);
  servo.write(pos);
  
  Logln("Pen set to " + String(percent) + " percent height");
}