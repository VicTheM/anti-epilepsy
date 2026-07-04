
#include "anti_epilepsy.h"

// Servo microneedleServo;
Adafruit_MPU6050 mpu;
MPU6050Data mpuData;


void setup() {
  Serial.begin(115200);
  delay(1000);

  if (!initMPU6050(mpu)) {
    Serial.println("Error initializing Accelerometer");
    delay(10);
  }
  else {
    Serial.println("MPU INITIALIZED SUCCESSFULLY");
  }
  Serial.println("Setup Completed");
}

void loop() {
  ;
}