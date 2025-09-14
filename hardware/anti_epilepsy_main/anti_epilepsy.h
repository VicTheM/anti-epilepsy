#ifndef ANTI_EPILEPSY_H
#define ANTI_EPILEPSY_H

// Avoid multiple definitions
#ifdef I2C_BUFFER_LENGTH
#undef I2C_BUFFER_LENGTH
#endif

#include <Arduino.h>
#include <ESP32Servo.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// Pin Assignments
const int PUMP_PIN = 25;
const int INDICATOR_PIN = 18;
const int BUZZER_PIN = 17;
const int DEBUG_SWITCH_PIN = 19;
const int STATE_PIN = 16;

// Constants for servo control
#define SERVO_PIN 13
#define SERVO_MIN 0
#define SERVO_MAX 180
#define SERVO_STEP 1
#define SERVO_DELAY 15
#define RATE_SIZE 4
#define PUMP_DELAY 700 // milliseconds

// Thresholds for seizure detection
#define ACCEL_THRESHOLD 15.0    // m/s^2
#define GYRO_THRESHOLD 3.0      // rad/s
#define BPM_UPPER_THRESHOLD 120       // beats per minute
#define BPM_LOWER_THRESHOLD 60         // beats per minute
#define NO_FINGER_THRESHOLD 50000 // IR value below which no finger is detected

// Debug mode configuration
#define DEBUG_AP_SSID "AntiEpilepsy"
#define DEBUG_AP_PASSWORD "debug123"
#define DEBUG_SERVER_PORT 80
#define CLICK_DEBOUNCE_MS 180 // Minimum time in ms between two distinct press detections
#define MULTI_CLICK_TIMEOUT_MS 1000 // Time in ms after a press to wait for more presses

/**
  * @brief Structure to hold button state and click count.
  * Members are volatile because they are modified within an ISR.
  */
struct Button {
    volatile unsigned long lastPressTime; // Time of the last detected press
    volatile unsigned int nPresses;     // Number of rapid presses within the timeout window
};

// Device states
enum DeviceState {
  NORMAL_MODE,
  DEBUG_MODE
};


// Struct to hold MPU6050 sensor data
struct MPU6050Data {
  float ax, ay, az;   // acceleration (m/s^2)
  float gx, gy, gz;   // gyroscope (rad/s)
  float temp;         // temperature (°C)
};

// Struct to hold MAX30105 data
struct MAX30105Data {
  long irValue;
  float bpm;
  int avgBpm;
  bool fingerDetected;
};

// Function prototypes
void moveServo(int x, int s, int y, Servo &myServo);
void resetServo(Servo &myServo);

bool initMPU6050(Adafruit_MPU6050 &mpu);
void readMPU6050(Adafruit_MPU6050 &mpu, MPU6050Data &data);

bool initMAX30105(MAX30105 &particleSensor);
void readMAX30105(MAX30105 &particleSensor, MAX30105Data &data);

void onPump();
void offPump();

void onAlarm();
void offAlarm();

void onIndicator();
void offIndicator();

bool checkForSeizure(const MPU6050Data &mpu, const MAX30105Data &heartbeat);
void administerDrug(Servo &myServo);

// Debug mode web server functions
void initDebugMode();
void endDebugMode();
void handleRoot(AsyncWebServerRequest *request);
void handleAlarm(AsyncWebServerRequest *request);
void handleIndicator(AsyncWebServerRequest *request);
void handleServo(AsyncWebServerRequest *request);
void handlePump(AsyncWebServerRequest *request);
void handleAdministerDrug(AsyncWebServerRequest *request);
String generateDebugHTML();
String getStateParam(AsyncWebServerRequest *request);

int getAndResetPressCount();
void onButtonInterrupt();

#endif