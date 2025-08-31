#ifndef ANTI_EPILEPSY_H
#define ANTI_EPILEPSY_H

#include <Arduino.h>
#include <Servo.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

// Constants for servo control
#define SERVO_PIN 9
#define SERVO_MIN 0
#define SERVO_MAX 180
#define SERVO_STEP 1
#define SERVO_DELAY 15

// Struct to hold MPU6050 sensor data
struct MPU6050Data {
  float ax, ay, az;   // acceleration (m/s^2)
  float gx, gy, gz;   // gyroscope (rad/s)
  float temp;         // temperature (°C)
};

// Struct to hold MAX30102 data
struct MAX30102Data {
  long irValue;
  float bpm;
  int avgBpm;
  bool fingerDetected;
};

// Function prototypes
void moveServo(int x, int s, int y, Servo &myServo);
void resetServo(Servo &myServo);

bool initMPU6050(Adafruit_MPU6050 &mpu);
bool readMPU6050(Adafruit_MPU6050 &mpu, MPU6050Data &data);

bool initMAX30102(MAX30102 &particleSensor);
bool readMAX30102(MAX30102 &particleSensor, MAX30102Data &data);

#endif