#pragma once

typedef struct Motor {
  int step;
  int dir;
  int enabled;
} Motor;

extern Motor motor1;
extern Motor motor2;

const int stepsPerRevolution = 20;
const int resolution = 16;

const int maxSteps = 4240;
const int maxRangeMM = 40;
const int stepsPerMM = maxSteps/maxRangeMM;

const int maxSpeed = 4800; //mm/min
const int normalSpeed = 1200;

void motorSetup();
void motorStep(int);
void motorSetSpeed(int);
void motorPower(bool);
void motorPower();
void plotLine(double, double);
void lineHigh(int, int, int, int);
void lineLow(int, int, int, int);
