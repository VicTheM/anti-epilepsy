// anti-epilepsy.cpp

#include "anti-epilepsy.h"

byte rates[RATE_SIZE]; // Array of heart rate values
byte rateSpot = 0;
long lastBeat = 0; // Time at which the last beat occurred


/**
 * Function to move servo to angle x, at speed s (degrees per second)
 * wait y ms, then return to initial position.
 * Parameters:
 *   x: target angle (0-180)
 *   s: speed in degrees per second
 *   y: wait time at target in milliseconds
 *   myServo: reference to Servo object
 */
void moveServo(int x, int s, int y, Servo &myServo) {
  int currentPos = myServo.read();
  int step = (x > currentPos) ? SERVO_STEP : -SERVO_STEP;
  int delayTime = 1000 / s;

  // Move to target angle
  for (int pos = currentPos; pos != x; pos += step) {
    myServo.write(pos);
    delay(delayTime);
  }
  myServo.write(x);

  delay(y); // wait at target

  // Return to initial position
  step = (currentPos > x) ? SERVO_STEP : -SERVO_STEP;
  for (int pos = x; pos != currentPos; pos += step) {
    myServo.write(pos);
    delay(delayTime);
  }
  myServo.write(currentPos);
}

/**
 * Function to return directly to initial position (0°).
 * Parameters:
 *   myServo: reference to Servo object
 */
void resetServo(Servo &myServo) {
  myServo.write(SERVO_MIN);
}

/**
 * Function to initialize MPU6050 sensor.
 * Parameters:
 *   mpu: reference to Adafruit_MPU6050 object
 * Returns:
 *   true if initialization is successful, false otherwise
 */
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
}


// Initialize MAX30105
bool initMAX30105(MAX30105 &particleSensor) {
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 was not found. Please check wiring/power.");
    return false;
  }

  Serial.println("MAX30105 Found! Place your finger on the sensor.");

  particleSensor.setup();                 // Default configuration
  particleSensor.setPulseAmplitudeRed(0x0A);   // Red LED low
  particleSensor.setPulseAmplitudeGreen(0);    // Green LED off

  return true;
}


// Read data into struct (passed by reference)
void readMAX30105(MAX30105 &particleSensor, MAX30105Data &data) {
  data.irValue = particleSensor.getIR();

  if (checkForBeat(data.irValue)) {
    // Beat detected
    long delta = millis() - lastBeat;
    lastBeat = millis();

    float beatsPerMinute = 60 / (delta / 1000.0);

    // Valid heart rate range
    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;

      int sum = 0;
      for (byte x = 0; x < RATE_SIZE; x++) {
        sum += rates[x];
      }
      data.avgBpm = sum / RATE_SIZE;
      data.bpm = beatsPerMinute;
    }
  }

  // Finger detection (IR drops low if no finger)
  data.fingerDetected = (data.irValue > 50000);
}