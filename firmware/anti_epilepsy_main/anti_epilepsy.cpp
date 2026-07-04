// anti-epilepsy.cpp

#include "anti_epilepsy.h"

bool initMPU6050(Adafruit_MPU6050 &mpu) {
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    return false;
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  return true;
}


/**
 * Function to read MPU6050 data into struct (passed by reference)
 * Parameters:
 *   data: reference to MPU6050Data struct to hold sensor readings
*/
void readMPU6050(Adafruit_MPU6050 &mpu, MPU6050Data &data) {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  data.ax = a.acceleration.x;
  data.ay = a.acceleration.y;
  data.az = a.acceleration.z;

  data.gx = g.gyro.x;
  data.gy = g.gyro.y;
  data.gz = g.gyro.z;

  data.temp = temp.temperature;


  // Debug output
  Serial.print("MPU6050 - Accel (m/s^2): ");
  Serial.print(data.ax); Serial.print(", ");
  Serial.print(data.ay); Serial.print(", ");
  Serial.print(data.az); Serial.print(" | ");
  Serial.print("Gyro (rad/s): ");
  Serial.print(data.gx); Serial.print(", ");
  Serial.print(data.gy); Serial.print(", ");
  Serial.print(data.gz); Serial.print(" | ");
  Serial.print("Temp (°C): ");
  Serial.println(data.temp);
}