// anti-epilepsy.cpp

#include "anti_epilepsy.h"

byte rates[RATE_SIZE];          // Array of heart rate values
byte rateSpot = 0;
long lastBeat = 0;              // Time at which the last beat occurred


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


/* Function to initialize MAX30105 sensor.
 * Parameters:
 *   particleSensor: reference to MAX30105 object
 * Returns:
 *   true if initialization is successful, false otherwise
 */
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


/** * Function to read MAX30105 data into struct (passed by reference)
 * Parameters:
 *   particleSensor: reference to MAX30105 object
 *   data: reference to MAX30105Data struct to hold sensor readings
 */
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

/**
  * Function to turn on the pump (set PUMP_PIN HIGH)
  */
void onPump() {
  digitalWrite(PUMP_PIN, HIGH);
  Serial.println("Pump ON");
}

/**
  * Function to turn off the pump (set PUMP_PIN LOW)
  */
void offPump() {
  digitalWrite(PUMP_PIN, LOW);
  Serial.println("Pump OFF");
}

/**
  * Function to turn on the alarm (set BUZZER_PIN HIGH)
  */
void onAlarm() {
  digitalWrite(BUZZER_PIN, HIGH);
  Serial.println("Alarm ON");
}

/**
  * Function to turn off the alarm (set BUZZER_PIN LOW)
  */
void offAlarm() {
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("Alarm OFF");
}

/**
  * Function to turn on the indicator (set INDICATOR_PIN HIGH)
  */
void onIndicator() {
  digitalWrite(INDICATOR_PIN, HIGH);
  Serial.println("Indicator ON");
}

/**
  * Function to turn off the indicator (set INDICATOR_PIN LOW)
  */
void offIndicator() {
  digitalWrite(INDICATOR_PIN, LOW);
  Serial.println("Indicator OFF");
}

/**
 * Function to check for seizure based on MPU6050 and MAX30105 data.
 * Parameters:
 *   mpu: reference to MPU6050Data struct with sensor readings
 *   heartbeat: reference to MAX30105Data struct with heart rate readings
 * Returns:
 *   true if seizure is detected, false otherwise
 */
bool checkForSeizure(const MPU6050Data &mpu, const MAX30105Data &heartbeat) {
  // Simple heuristic: if heart rate is very high or very low, and MPU6050 detects strong motion
  if ((heartbeat.avgBpm > BPM_UPPER_THRESHOLD || heartbeat.avgBpm < BPM_LOWER_THRESHOLD) &&
      (abs(mpu.ax) > ACCEL_THRESHOLD || abs(mpu.ay) > ACCEL_THRESHOLD || abs(mpu.az) > ACCEL_THRESHOLD ||
       abs(mpu.gx) > GYRO_THRESHOLD || abs(mpu.gy) > GYRO_THRESHOLD || abs(mpu.gz) > GYRO_THRESHOLD) &&
      heartbeat.irValue < NO_FINGER_THRESHOLD) {
    Serial.println("Seizure detected!");
    return true;
  }
  return false;
}

/**
 * Function to administer drug via microneedle servo mechanism.
 * Sequence: turn on pump, beep 3 times, blink 3 times, turn off pump,
 * then move servo 3 times while continuously beeping and blinking.
 * Parameters:
 *   myServo: reference to Servo object controlling the microneedle
 */
void administerDrug(Servo &myServo) {
  Serial.println("DRUG ADMINISTRATION SEQUENCE STARTED");
  
  // Step 1: Pump serum out
  onPump();
  delay(500);
  offPump();
  
  // Step 2: Beep alarm 3 times, Step 3: Blink light 3 times
  for (int i = 0; i < 3; i++) {
    onAlarm();
    onIndicator();
    delay(300);
    offAlarm();
    offIndicator();
    delay(200);
  }
  
  // Step 4: Move servo 3 times while continuously beeping and blinking
  Serial.println("Starting microneedle injection sequence");
  
  for (int cycle = 0; cycle < 3; cycle++) {
    Serial.print("Injection cycle ");
    Serial.println(cycle + 1);
    
    // Use moveServo function: move to 180°, speed 10°/s, wait 200ms at target
    // This automatically returns to initial position (0°)
    moveServo(180, 10, 200, myServo);
    
    for (int i = 0; i < 3; i++) {
      onAlarm();
      onIndicator();
      delay(300);
      offAlarm();
      offIndicator();
      delay(200);
    }
    
  }
  
  // Ensure servo is at home position and all outputs are off
  myServo.write(0);
  offAlarm();
  offIndicator();
  offPump();
  
  Serial.println("DRUG ADMINISTRATION SEQUENCE COMPLETED");
}

// Global variables for web server and debug mode
AsyncWebServer server(DEBUG_SERVER_PORT);
extern Servo microneedleServo; // Reference to servo from main file


/**
 * Initialize debug mode - create access point and start web server
 */
void initDebugMode() {
  Serial.println("Entering DEBUG MODE");
  Serial.println("Creating WiFi Access Point...");
  
  // Create WiFi Access Point
  WiFi.softAP(DEBUG_AP_SSID, DEBUG_AP_PASSWORD);
  IPAddress IP = WiFi.softAPIP();
  
  Serial.print("AP IP address: ");
  Serial.println(IP);
  Serial.print("SSID: ");
  Serial.println(DEBUG_AP_SSID);
  Serial.print("Password: ");
  Serial.println(DEBUG_AP_PASSWORD);
  
  // Setup async web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/alarm", HTTP_GET, handleAlarm);
  server.on("/indicator", HTTP_GET, handleIndicator);
  server.on("/servo", HTTP_GET, handleServo);
  server.on("/pump", HTTP_GET, handlePump);
  server.on("/administer", HTTP_GET, handleAdministerDrug);
  
  server.begin();
  Serial.println("Async debug web server started");
}

// Helper to read ?state=on/off
String getStateParam(AsyncWebServerRequest *request) {
  if (request->hasParam("state")) {
    return request->getParam("state")->value();
  }
  return "";
}

/**
 * Handle alarm button - set alarm on/off via query string, else toggle
 */
void handleAlarm(AsyncWebServerRequest *request) {
  static bool alarmState = false;
  String state = getStateParam(request);

  if (state == "on") {
    onAlarm();
    alarmState = true;
    request->send(200, "text/plain", "Alarm ON");
  } else if (state == "off") {
    offAlarm();
    alarmState = false;
    request->send(200, "text/plain", "Alarm OFF");
  } else {
    // toggle if no state given
    alarmState = !alarmState;
    if (alarmState) {
      onAlarm();
      request->send(200, "text/plain", "Alarm ON");
    } else {
      offAlarm();
      request->send(200, "text/plain", "Alarm OFF");
    }
  }

  Serial.println("Debug: Alarm handler executed");
}

/**
 * Handle indicator button - set indicator on/off via query string, else toggle
 */
void handleIndicator(AsyncWebServerRequest *request) {
  static bool indicatorState = false;
  String state = getStateParam(request);

  if (state == "on") {
    onIndicator();
    indicatorState = true;
    request->send(200, "text/plain", "Indicator ON");
  } else if (state == "off") {
    offIndicator();
    indicatorState = false;
    request->send(200, "text/plain", "Indicator OFF");
  } else {
    indicatorState = !indicatorState;
    if (indicatorState) {
      onIndicator();
      request->send(200, "text/plain", "Indicator ON");
    } else {
      offIndicator();
      request->send(200, "text/plain", "Indicator OFF");
    }
  }

  Serial.println("Debug: Indicator handler executed");
}

/**
 * Handle servo - just run as before (no on/off concept here)
 */
void handleServo(AsyncWebServerRequest *request) {
  Serial.println("Debug: Servo test initiated via web interface");
  moveServo(90, 30, 1000, microneedleServo);
  request->send(200, "text/plain", "Servo moved to 90° and back");
}

/**
 * Handle pump button - set pump on/off via query string, else toggle
 */
void handlePump(AsyncWebServerRequest *request) {
  static bool pumpState = false;
  String state = getStateParam(request);

  if (state == "on") {
    onPump();
    pumpState = true;
    request->send(200, "text/plain", "Pump ON");
  } else if (state == "off") {
    offPump();
    pumpState = false;
    request->send(200, "text/plain", "Pump OFF");
  } else {
    pumpState = !pumpState;
    if (pumpState) {
      onPump();
      request->send(200, "text/plain", "Pump ON");
    } else {
      offPump();
      request->send(200, "text/plain", "Pump OFF");
    }
  }

  Serial.println("Debug: Pump handler executed");
}

/**
 * Handle administer drug - no state, just trigger sequence
 */
void handleAdministerDrug(AsyncWebServerRequest *request) {
  Serial.println("Debug: Drug administration initiated via web interface");
  administerDrug(microneedleServo);
  request->send(200, "text/plain", "Drug administration sequence completed");
}


void endDebugMode() {
  server.end();
  WiFi.softAPdisconnect(true);
}

String generateDebugHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Debug Interface</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; background: #f4f4f4; }
    h1 { margin-top: 20px; }
    .btn {
      display: inline-block;
      padding: 12px 20px;
      margin: 8px;
      font-size: 16px;
      cursor: pointer;
      border: none;
      border-radius: 6px;
      background: #007BFF;
      color: white;
      transition: background 0.3s;
    }
    .btn:hover { background: #0056b3; }
    #log {
      margin-top: 20px;
      padding: 10px;
      background: #fff;
      border: 1px solid #ccc;
      width: 80%;
      max-width: 500px;
      margin-left: auto;
      margin-right: auto;
      text-align: left;
      font-family: monospace;
      white-space: pre-wrap;
    }
  </style>
</head>
<body>
  <h1>ESP32 Debug Interface</h1>

  <h2>Alarm</h2>
  <button class="btn" onclick="sendRequest('/alarm?state=on')">Alarm ON</button>
  <button class="btn" onclick="sendRequest('/alarm?state=off')">Alarm OFF</button>

  <h2>Indicator</h2>
  <button class="btn" onclick="sendRequest('/indicator?state=on')">Indicator ON</button>
  <button class="btn" onclick="sendRequest('/indicator?state=off')">Indicator OFF</button>

  <h2>Servo</h2>
  <button class="btn" onclick="sendRequest('/servo')">Move Servo</button>

  <h2>Pump</h2>
  <button class="btn" onclick="sendRequest('/pump?state=on')">Pump ON</button>
  <button class="btn" onclick="sendRequest('/pump?state=off')">Pump OFF</button>

  <h2>Drug Administration</h2>
  <button class="btn" onclick="sendRequest('/administer')">Administer Drug</button>

  <div id="log">Logs will appear here...</div>

  <script>
    function sendRequest(endpoint) {
      fetch(endpoint)
        .then(response => response.text())
        .then(data => {
          log("[" + new Date().toLocaleTimeString() + "] " + endpoint + " -> " + data);
        })
        .catch(err => {
          log("Error: " + err);
        });
    }

    function log(message) {
      const logDiv = document.getElementById("log");
      logDiv.textContent = message + "\n" + logDiv.textContent;
    }
  </script>
</body>
</html>
  )rawliteral";
}
