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
  Serial.print("MAX30105 - IR Value: ");
  Serial.print(data.irValue);

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

      Serial.print(" | BPM: ");
      Serial.print(beatsPerMinute);
    }
    else {
      Serial.print(" | BPM: --");
    }
  }

  // Finger detection (IR drops low if no finger)
  data.fingerDetected = (data.irValue >  IR_CAP_VALUE);
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

void handleRoot(AsyncWebServerRequest *request) {
  request->send(200, "text/html", generateDebugHTML());
  Serial.println("Debug: Root page served");
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
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Antieplilepsy</title>
  <style>
    :root {
      --dark-bg: #1a1a1a;
      --card-bg: #2a2a2a;
      --indigo: #4b0082;
      --red: #dc143c;
      --text-color: #f0f0f0;
      --transition-speed: 0.3s;
    }

    body {
      font-family: Arial, sans-serif;
      text-align: center;
      background: var(--dark-bg);
      color: var(--text-color);
      margin: 0;
      padding: 0;
      display: flex;
      flex-direction: column;
      justify-content: center;
      align-items: center;
      height: 100vh;
      overflow: hidden; /* Prevent scrolling */
    }

    h1, h2 {
      color: var(--text-color);
      margin-top: 20px;
    }

    .main-grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      grid-template-rows: auto auto;
      gap: 20px;
      padding: 20px;
      width: 90%;
      max-width: 1200px;
      height: 100%;
      align-items: center;
      justify-content: center;
    }

    .section-card {
      background: var(--card-bg);
      padding: 20px;
      border-radius: 10px;
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.4);
      display: flex;
      flex-direction: column;
      justify-content: center;
      height: 100%;
    }

    .btn-group {
      display: flex;
      flex-wrap: wrap;
      justify-content: center;
      gap: 10px;
      margin-top: 10px;
    }

    .btn {
      flex: 1 1 45%;
      padding: 12px 20px;
      font-size: 16px;
      cursor: pointer;
      border: none;
      border-radius: 6px;
      color: white;
      transition: background var(--transition-speed), transform var(--transition-speed);
      text-transform: uppercase;
    }

    .btn-indigo {
      background: var(--indigo);
    }
    .btn-indigo:hover {
      background: #3b006a;
      transform: translateY(-2px);
    }

    .btn-red {
      background: var(--red);
    }
    .btn-red:hover {
      background: #a91030;
      transform: translateY(-2px);
    }

    .slider-container {
      margin-top: 20px;
      text-align: left;
    }

    .slider-container label {
      display: block;
      margin-bottom: 5px;
      font-weight: bold;
    }

    .slider-value {
      float: right;
    }

    .slider {
      width: 100%;
      -webkit-appearance: none;
      height: 10px;
      background: #555;
      outline: none;
      border-radius: 5px;
    }

    .slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 20px;
      height: 20px;
      background: var(--indigo);
      cursor: pointer;
      border-radius: 50%;
    }

    #log-canvas {
      position: fixed;
      top: 50%;
      left: 50%;
      transform: translate(-50%, -50%);
      width: 80%;
      max-width: 600px;
      height: 70%;
      max-height: 500px;
      background: var(--card-bg);
      border-radius: 10px;
      box-shadow: 0 8px 16px rgba(0, 0, 0, 0.6);
      padding: 20px;
      display: flex;
      flex-direction: column;
      z-index: 2000;
      opacity: 0;
      visibility: hidden;
      transition: opacity var(--transition-speed), visibility var(--transition-speed);
    }

    #log-canvas.visible {
      opacity: 1;
      visibility: visible;
    }

    #log-canvas h2 {
      margin-top: 0;
      margin-bottom: 10px;
    }

    #log {
      flex-grow: 1;
      overflow-y: scroll;
      padding: 10px;
      border: 1px solid #444;
      border-radius: 5px;
      text-align: left;
      font-family: monospace;
      white-space: pre-wrap;
      font-size: 14px;
      background: #111;
    }

    .close-btn {
      position: absolute;
      top: 10px;
      right: 10px;
      background: none;
      border: none;
      color: var(--text-color);
      font-size: 24px;
      cursor: pointer;
    }

    .show-log-btn {
      position: fixed;
      bottom: 20px;
      right: 20px;
      padding: 15px;
      border-radius: 50%;
      background: var(--indigo);
      color: white;
      border: none;
      cursor: pointer;
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.4);
      z-index: 1000;
    }

    @media (max-width: 768px) {
      body {
        height: auto;
        overflow-y: auto;
      }
      .main-grid {
        grid-template-columns: 1fr;
        height: auto;
      }
    }
  </style>
</head>
<body>
  <h1>Manual Control Board</h1>

  <div class="main-grid">
    <div class="section-card">
      <h2>Alarm & Indicator</h2>
      <div class="btn-group">
        <button class="btn btn-indigo" onclick="sendRequest('/alarm?state=on')">Alarm ON</button>
        <button class="btn btn-red" onclick="sendRequest('/alarm?state=off')">Alarm OFF</button>
        <button class="btn btn-indigo" onclick="sendRequest('/indicator?state=on')">Indicator ON</button>
        <button class="btn btn-red" onclick="sendRequest('/indicator?state=off')">Indicator OFF</button>
      </div>
    </div>

    <div class="section-card">
      <h2>Microneedle & Pump</h2>
      <div class="btn-group">
        <button class="btn btn-indigo" onclick="sendRequest('/servo')">Move Servo</button>
        <button class="btn btn-indigo" onclick="sendRequest('/pump?state=on')">Pump ON</button>
        <button class="btn btn-red" onclick="sendRequest('/pump?state=off')">Pump OFF</button>
      </div>
    </div>

    <div class="section-card" style="grid-column: 1 / span 2;">
      <h2>Simulate Symptoms</h2>
      <div class="slider-container">
        <label for="heart-rate">Heart Rate: <span id="heart-rate-val" class="slider-value">100 bpm</span></label>
        <input type="range" min="0" max="200" value="100" class="slider" id="heart-rate">
      </div>
      <div class="slider-container">
        <label for="vibration">Vibration: <span id="vibration-val" class="slider-value">10 m/s²</span></label>
        <input type="range" min="0" max="20" step="0.1" value="10" class="slider" id="vibration">
      </div>
      <div class="slider-container">
        <label for="orientation">Orientation: <span id="orientation-val" class="slider-value">45°</span></label>
        <input type="range" min="0" max="90" value="45" class="slider" id="orientation">
      </div>
    </div>

    <div class="section-card" style="grid-column: 1 / span 2;">
      <h2>Drug Administration</h2>
      <div class="btn-group">
        <button class="btn btn-red" onclick="sendRequest('/administer')" id="administer-btn">Administer Drug</button>
      </div>
    </div>
  </div>

  <div id="log-canvas">
    <button class="close-btn" onclick="closeLog()">×</button>
    <h2>Event Log</h2>
    <div id="log"></div>
  </div>

  <button class="show-log-btn" onclick="openLog()">Show Log</button>

  <script>
    const heartRateSlider = document.getElementById('heart-rate');
    const vibrationSlider = document.getElementById('vibration');
    const orientationSlider = document.getElementById('orientation');

    const heartRateValue = document.getElementById('heart-rate-val');
    const vibrationValue = document.getElementById('vibration-val');
    const orientationValue = document.getElementById('orientation-val');

    heartRateSlider.oninput = function() {
      heartRateValue.textContent = this.value + ' bpm';
      checkThresholds();
    };

    vibrationSlider.oninput = function() {
      vibrationValue.textContent = this.value + ' m/s²';
      checkThresholds();
    };

    orientationSlider.oninput = function() {
      orientationValue.textContent = this.value + '°';
      checkThresholds();
    };

    function checkThresholds() {
      const heartRate = parseInt(heartRateSlider.value);
      const orientation = parseInt(orientationSlider.value);
      const vibration = parseFloat(vibrationSlider.value);
      
      const isCritical = (heartRate < 60 || heartRate > 120) && orientation < 45 && vibration > 15;

      if (isCritical) {
        log("CRITICAL THRESHOLDS MET! Administering drug automatically.");
        sendRequest('/administer');
      }
    }

    function sendRequest(endpoint) {
      log("Sending request to: " + endpoint);
      // Simulate an asynchronous fetch request since we are offline
      return new Promise(resolve => {
        setTimeout(() => {
          const simulatedResponse = "OK";
          log("Response from " + endpoint + ": " + simulatedResponse);
          resolve(simulatedResponse);
        }, 500); // Simulate a 500ms network delay
      }).catch(err => {
        log("Error: " + err);
      });
    }

    function log(message) {
      const logDiv = document.getElementById("log");
      const timestamp = new Date().toLocaleTimeString('en-US', { hour12: false });
      const newMessage = `<p><strong>[${timestamp}]</strong> ${message}</p>`;
      logDiv.innerHTML = newMessage + logDiv.innerHTML;
    }

    function openLog() {
      document.getElementById('log-canvas').classList.add('visible');
    }

    function closeLog() {
      document.getElementById('log-canvas').classList.remove('visible');
    }
  </script>
</body>
</html>
  )rawliteral";
}
