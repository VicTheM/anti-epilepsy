
#include "anti_epilepsy.h"

Servo microneedleServo;
Adafruit_MPU6050 mpu;
MPU6050Data mpuData;
MAX30105 particleSensor;
MAX30105Data heartData;

Button stateButton = {0, 0};

DeviceState currentState = NORMAL_MODE;
bool debugModeInitialized = false;

void setup() {
  Serial.begin(115200);
  delay(1000);

  int n = 0;
  if (!initMAX30105(particleSensor)) {
    Serial.println("Error initializing heart beat rate sensor");
    while (n++ != 20) delay(10);
  }
  n = 0;

  if (!initMPU6050(mpu)) {
    Serial.println("Error initializing Accelerometer");
    while (n++ != 20) delay(10);
  }
  n = 0;

  pinMode(INDICATOR_PIN, OUTPUT);
  digitalWrite(INDICATOR_PIN, LOW);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);

  // Setup debug switch pin
  pinMode(DEBUG_SWITCH_PIN, INPUT_PULLUP);

  microneedleServo.attach(SERVO_PIN);
  microneedleServo.write(SERVO_MIN);

  readMPU6050(mpu, mpuData);
  readMAX30105(particleSensor, heartData);

  attachInterrupt(digitalPinToInterrupt(STATE_PIN), onButtonInterrupt, FALLING);
  Serial.println("Setup Completed");
}

void loop() {

  // Configuring state
  int count = getAndResetPressCount();
  if (count == 1 && currentState == NORMAL_MODE) {
    // Switch to debug mode
    currentState = DEBUG_MODE;
    initDebugMode();
    debugModeInitialized = true;
    Serial.println("Debug switch activated - switching to DEBUG MODE");
  } else if (count == 2 && currentState == DEBUG_MODE) {
    // Switch back to normal mode
    currentState = NORMAL_MODE;
    endDebugMode();
    debugModeInitialized = false;
    Serial.println("Debug switch deactivated - switching to NORMAL MODE");
  }


  if (currentState == DEBUG_MODE) {
    ;
  } else {
    // NORMAL MODE: Seizure monitoring
    readMPU6050(mpu, mpuData);
    readMAX30105(particleSensor, heartData);
    
    if (checkForSeizure(mpuData, heartData)) {
      Serial.println("SEIZURE DETECTED - Administering medication");
      administerDrug(microneedleServo);
    }
  }
  delay(100);
}


// ISR function - called directly by the hardware interrupt
void IRAM_ATTR onButtonInterrupt() {
    unsigned long currentTime = millis();
    if (currentTime - stateButton.lastPressTime > CLICK_DEBOUNCE_MS) {
        stateButton.lastPressTime = currentTime;
        stateButton.nPresses = stateButton.nPresses + 1;
        // Serial.printf("[PeripheralHandler] ISR: Press detected, count = %u\n", stateButton.nPresses);
    }
}

int getAndResetPressCount() {
  unsigned int clicks = 0;
  if (stateButton.nPresses > 0) {
    unsigned long currentTime = millis();
    if (currentTime - stateButton.lastPressTime > MULTI_CLICK_TIMEOUT_MS) {
        // Disable interrupts briefly to safely read and reset volatile variables
        noInterrupts();
        clicks = stateButton.nPresses;
        interrupts();

        // Serial.printf("[PeripheralHandler] Detected %u clicks.\n", clicks);
        return clicks;
    }
    else {
        delay(MULTI_CLICK_TIMEOUT_MS);
        clicks = stateButton.nPresses; // Capture the current count
        return clicks;
    }

    stateButton.nPresses = 0;
  }
  return clicks;
}