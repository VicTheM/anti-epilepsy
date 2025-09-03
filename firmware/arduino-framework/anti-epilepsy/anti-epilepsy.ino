#include <Arduino.h>
#include <Servo.h>
#include "anti-epilepsy.h"

Servo microneedleServo;
Adafruit_MPU6050 mpu;
MPU6050Data mpuData;
MAX30102Data heartData;


void setup() {
  Serial.begin(9600);

  microneedleServo.attach(SERVO_PIN);
  resetServo(microneedleServo);

  if (!initMPU6050(mpu)) {
    while (1) {
      delay(10);
    }
  }

  if (!initMAX30102()) {
    while (1) delay(10);
  }

  readMPU6050(mpu, mpuData);
}

void loop() {
  // Tests
  resetServo(microneedleServo);
  moveServo(90, 30, 1000, microneedleServo);
  delay(2000);
  resetServo(microneedleServo);
  delay(2000);

  readMPU6050(mpu, mpuData);
  Serial.print("Accel (m/s^2) X:");
  Serial.print(mpuData.ax);
  Serial.print(" Y:");
  Serial.print(mpuData.ay);
  Serial.print(" Z:");
  Serial.println(mpuData.az);

  Serial.print("Gyro (rad/s) X:");
  Serial.print(mpuData.gx);
  Serial.print(" Y:");
  Serial.print(mpuData.gy);
  Serial.print(" Z:");
  Serial.println(mpuData.gz);

  Serial.print("Temp: ");
  Serial.print(mpuData.temp);
  Serial.println(" °C");
  Serial.println("------------------------");

  readMAX30102(particleSensor, heartData);
  Serial.print("IR=");
  Serial.print(heartData.irValue);
  Serial.print(", BPM=");
  Serial.print(heartData.bpm);
  Serial.print(", Avg BPM=");
  Serial.print(heartData.avgBpm);

  if (!heartData.fingerDetected) {
    Serial.print("  No finger?");
  }

  Serial.println();

  delay(500);
}