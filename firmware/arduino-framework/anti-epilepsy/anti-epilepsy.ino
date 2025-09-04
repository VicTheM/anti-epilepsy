// anti-epilepsy.ino

#include <Arduino.h>
#include "anti-epilepsy.h"

Servo microneedleServo;
// Adafruit_MPU6050 mpu;
// MPU6050Data mpuData;
MAX30105 particleSensor;
MAX30105Data heartData;


void setup() {
  Serial.begin(9600);
  delay(2000);

  microneedleServo.attach(SERVO_PIN);
  // microneedleServo.write(0);
  resetServo(microneedleServo);

  // if (!initMPU6050(mpu)) {
  //   while (1) {
  //     delay(10);
  //   }
  // }

  if (!initMAX30105(particleSensor)) {
    Serial.println("Error!");
    while (1) delay(10);
  }

  Serial.println("Setup Completed");

  // readMPU6050(mpu, mpuData);
  readMAX30105(particleSensor, heartData);
}

void loop() {
  // Tests
  // resetServo(microneedleServo);
  // moveServo(180, 30, 1000, microneedleServo);
  // delay(2000);
  // resetServo(microneedleServo);
  // delay(2000);

  microneedleServo.write(0);
  delay(1000);
  microneedleServo.write(90);
  delay(1000);
  microneedleServo.write(180);
  delay(1000);

  // if (!heartData.fingerDetected) {
  //   Serial.print("  No finger?");
  // }
  readMAX30105(particleSensor, heartData);
  Serial.print("IR=");
  Serial.print(heartData.irValue);
  Serial.print(", BPM=");
  Serial.print(heartData.bpm);
  Serial.print(", Avg BPM=");
  Serial.print(heartData.avgBpm);
  // readMPU6050(mpu, mpuData);
  // Serial.print("Accel (m/s^2) X:");
  // Serial.print(mpuData.ax);
  // Serial.print(" Y:");
  // Serial.print(mpuData.ay);
  // Serial.print(" Z:");
  // Serial.println(mpuData.az);

  // Serial.print("Gyro (rad/s) X:");
  // Serial.print(mpuData.gx);
  // Serial.print(" Y:");
  // Serial.print(mpuData.gy);
  // Serial.print(" Z:");
  // Serial.println(mpuData.gz);

  // Serial.print("Temp: ");
  // Serial.print(mpuData.temp);
  // Serial.println(" °C");
  // Serial.println("------------------------");

  Serial.println("--------------------------------------------------------------------");

  delay(500);
}