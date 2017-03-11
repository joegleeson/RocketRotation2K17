/**
   @Author: Yichen Xu
   @Date:   19-Feb-2017 23:02:34
   @Email:  ichi@u.northwestern.edu
   @Last modified by:   Yichen Xu
   @Last modified time: 19-Feb-2017 23:02:41
 */



#include "PID.h"

int prevEI;
int prevE = 720;
float turnLeft = 720;
int powerG;
int isLaunchGyroSet = 0;
float pE, iE, dE;


int calculatePID() {
    float u;
    int dt = currentData.time - prevData.time;
    // change this so that going across 0 and 360 doesnt involve a huge difference (degrees or radians?)

    // change in orientation = change in turn left

    double xdot = ((currentData.gyro.x - launchGyro) / (SENSORS_DPS_TO_RADS * 1000)) * dt;

    // double xdot = (currentData.gyro.x) / (SENSORS_DPS_TO_RADS * 1000) * dt;

  #if DEBUG
      Serial.print("    xDot:  ");
    Serial.println(xdot);
  #endif

    // set reference angular velocity
    // take angular velocity * dt, should be equal to dPos
    // subtract from turnLeft
    // *need to figure out which way to turn (oppose direction of motion?)
    // *or stop velcoity first, but thats harder
    // then we can use turnleft to do everything else.

    turnLeft = turnLeft - xdot;

    float error = calculateError();
    pE = error * kp;
    dE = (error - prevE)/dt;
    //dE = dt * (turnLeft - prevE);
    prevE = error;
    
    prevEI += error*dt;
    //prevEI += turnleft;
    
    if (prevEI > 100/ki) {
        prevEI = 100/ki;
    }
    if (prevEI < -100/ki) {
        prevEI = -100/ki;
    }
    iE = prevEI * ki;

    dE = kd*dE;

    u = iE + dE + pE;

    return u;
}

void outputMotor(int power) {
    powerG = power;
    #if DEBUG
    Serial.print("Power:   ");
    Serial.println(power);
    #endif
    analogWrite(motorPin, 255);
    analogWrite(torquePin, 0);
    if (power > 0) {

        analogWrite(directionPin, 255);
        analogWrite(speedPin, abs(power));
    }
    else {
        analogWrite(directionPin, 0);
        analogWrite(speedPin, power);
    }
}

void doTheThing(uint32_t timestamp) {
    #if DEBUG
    Serial.print(currentData.time - timestamp);
    Serial.print("    turnleft:  ");
    Serial.print(turnLeft);
    
    #endif
    if ((currentData.time - timestamp) > waitTime) {
        if (!isLaunchGyroSet) {
            launchGyro = currentData.gyro.x;
            isLaunchGyroSet = !isLaunchGyroSet;
        }

        int u = calculatePID();

        if (u < 0) {
            if ( u < -255) {
                u = -255;
            }
            outputMotor(u);
        }
        else {
            if (u > 255) {
                u = 255;
            }
            
            outputMotor(u);
        }
    }
}

float calculateError() {
  uint32_t timeElapsed = currentData.time - launchTimestamp-5000;
  float errorT = 720-(720/5000)*timeElapsed;
  Serial.print("timeElapsed: ");
  Serial.print(timeElapsed);
  if (errorT < 0) {
    errorT = 0;
}
  Serial.print("error: ");
  Serial.println(turnLeft - errorT);
  return turnLeft - errorT;

}

