/*
 * Copyright (c) 2014 Inventit Inc.
 */

#include <Sparki.h>
#include "math.h"

#define BPM 125
#define BEATS 4
#define MS_PER_BEAT 1000L * 60L / BPM
#define MS_PER_MEASURE MS_PER_BEAT * BEATS
#define STEPS_TO_AVOID 5

const int pitch = 1; // in cm
const float goalX = 12.5;
const float goalY = 9.0;
const int avoidanceDirection = goalY - goalX;
const float pi = 3.14159;
const int threshold = 200;
const int avoidanceActionLoop = 2;
const int offset = 5; // the length of the gripper and the front surface.

int direction = 180; // degree (not radian), 90 = north, 270 = south, 180 = west, 0 = east
float currentX = 83.5;
float currentY = 18.0;
int avoidanceSteps = 0;

float computeDirection(float nx, float ny) {
  const float t = atan2((ny - currentY), (nx - currentX));
  // convert to 0 to 360 range
  return (t > 0 ? t : (2 * pi + t)) * 360 / (2 * pi);
}

void move(int p, float d) {
  float t = pi * d / 180;
  float dx = p * cos(t);
  float dy = p * sin(t);
  
  currentX = currentX + dx;
  currentY = currentY + dy;
  
  Serial.print("currentX:");
  Serial.print(currentX);
  Serial.print(", currentY:");
  Serial.print(currentY);
  
  if (p > 0) {
    sparki.moveForward(p);
  } else if (p < 0) {
    sparki.moveBackward(-p); // must be positive
  }
  Serial.print("d:");
  Serial.println(d);
}

boolean somethingBlocksMe() {
  if (spaceFoundUnderneath() || wallFound()) {
    return true;
  }
  return false;
}

boolean wallFound() {
  const int cm = sparki.ping();
  if (cm == -1) {
    return false;
  }
  if (cm <= pitch + offset) {
    Serial.println("[ultrasonic]: found a wall!");
    return true;
  }
  return false;
}

boolean spaceFoundUnderneath() {
  if (sparki.edgeLeft() < threshold) {
    Serial.println("[edgeLeft]: found a hole!");
    return true;
  } else if (sparki.edgeRight() < threshold) {
    Serial.println("[edgeRight]: found a hole!");
    return true;
  }
  return false;
}

void stepBackAndTurn() {
  // step back
  move(-pitch, direction);
  
  if (avoidanceDirection < 0) {
    turn(direction - 45);
  } else {
    turn(direction + 45);
  }
}

void turn(int newDirection) {
  const int diff = newDirection + (newDirection < 0 ? direction : -direction);
  if (diff < 0) {
    Serial.print("Turn right! => ");
    Serial.println(-diff);
    sparki.moveRight(-diff);
  } else if (diff > 0) {
    Serial.print("Turn left! => ");
    Serial.println(-diff);
    sparki.moveLeft(diff);
  }
  direction = newDirection;
}


boolean arriveAtGoal() {
  if (abs(goalX - currentX) >= 1) {
    return false;
  }
  if (abs(goalY - currentY) >= 1) {
    return false;
  }
  return true;
}

void dance6() {
  for (int i = 0; i < 4; i++) {
    sparki.moveRight(10);
    sparki.moveLeft(20);
    sparki.moveRight(10);  
  }
  sparki.gripperOpen();
  delay(MS_PER_BEAT / 2);
  sparki.gripperClose();
  delay(MS_PER_BEAT / 2);
  sparki.gripperOpen();
  delay(MS_PER_BEAT / 2);
  sparki.gripperClose();
  delay(MS_PER_BEAT / 2);
  sparki.gripperStop();
}

void setup() {
  // straight face
  sparki.servo(SERVO_RIGHT);
  sparki.servo(SERVO_LEFT);
  sparki.servo(SERVO_CENTER);
}

void loop() {
  if (avoidanceSteps <= 0) {
    const int newDirection = (int) computeDirection(goalX, goalY);
    Serial.println(newDirection);
    turn(newDirection);
  } else {
    avoidanceSteps--;
  }
  move(pitch, direction);
  if (arriveAtGoal()) {
    dance6();
    end();
    return ; // never reached here
  }
  Serial.println("in loop");
  while (somethingBlocksMe()) {
    avoidanceSteps = STEPS_TO_AVOID;
    stepBackAndTurn();
  }
  if (arriveAtGoal()) {
    dance6();
    end();
    return ; // never reached here
  }
}

void end() {
  // infinite loop
  Serial.print("direction:");
  Serial.println(direction);
  Serial.print("currentX:");
  Serial.println(currentX);
  Serial.print("currentY:");
  Serial.println(currentY);
  Serial.print("goalX:");
  Serial.println(goalX);
  Serial.print("goalY:");
  Serial.println(goalY);
  
  randomSeed(millis());
  while (true) {
    sparki.RGB(random(101), random(101), random(101));
    delay(100);
  }
}

