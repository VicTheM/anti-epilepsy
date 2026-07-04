#ifndef ANTI_EPILEPSY_H
#define ANTI_EPILEPSY_H

// Avoid multiple definitions
#ifdef I2C_BUFFER_LENGTH
#undef I2C_BUFFER_LENGTH
#endif

#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <WiFi.h>

// Thresholds for seizure detection
#define ACCEL_THRESHOLD 15.0    // m/s^2
#define GYRO_THRESHOLD 3.0      // rad/s
#define BPM_UPPER_THRESHOLD 120       // beats per minute
#define BPM_LOWER_THRESHOLD 60         // beats per minute
#define NO_FINGER_THRESHOLD 5000000 // IR value below which no finger is detected
#define IR_CAP_VALUE 70000 // IR value above which finger is definitely detected


// Struct to hold MPU6050 sensor data
struct MPU6050Data {
  float ax, ay, az;   // acceleration (m/s^2)
  float gx, gy, gz;   // gyroscope (rad/s)
  float temp;         // temperature (°C)
};


bool initMPU6050(Adafruit_MPU6050 &mpu);

#endif