
#include "anti_epilepsy.h"

Servo microneedleServo;
Adafruit_MPU6050 mpu;
MPU6050Data mpuData;
MAX30105 particleSensor;
MAX30105Data heartData;

volatile Button stateButton = {0, 0};

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

  pinMode(SERVER_LIGHT_PIN, OUTPUT);
  digitalWrite(SERVER_LIGHT_PIN, LOW);

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

  attachInterrupt(digitalPinToInterrupt(DEBUG_SWITCH_PIN), onButtonInterrupt, FALLING);
  Serial.println("Setup Completed");
}

void loop() {

  // Configuring state
  int count = getAndResetPressCount();
  if (count > 1 && currentState == NORMAL_MODE) {
    // Switch to debug mode
    currentState = DEBUG_MODE;
    initDebugMode();
    debugModeInitialized = true;
    digitalWrite(SERVER_LIGHT_PIN, HIGH);
    Serial.println("Debug switch activated - switching to DEBUG MODE");
  } else if (count == 1 && currentState == DEBUG_MODE) {
    // Switch back to normal mode
    currentState = NORMAL_MODE;
    endDebugMode();
    debugModeInitialized = false;
    digitalWrite(SERVER_LIGHT_PIN, LOW);
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
      // administerDrug(microneedleServo);
    }
  }
  delay(100);
}


// ISR function - called directly by the hardware interrupt
void IRAM_ATTR onButtonInterrupt() {
  handleButtonPress();
}

void handleButtonPress() {
    unsigned long currentTime = millis();
    if (currentTime - stateButton.lastPressTime > CLICK_DEBOUNCE_MS) {
        stateButton.lastPressTime = currentTime;
        stateButton.nPresses = stateButton.nPresses + 1;
    }
}

int getAndResetPressCount() {
  unsigned int clicks = 0;
  if (stateButton.nPresses > 0) {
    unsigned long currentTime = millis();
    if (currentTime - stateButton.lastPressTime > MULTI_CLICK_TIMEOUT_MS) {
      noInterrupts();
      clicks = stateButton.nPresses;
      interrupts();
    }
    else {
      delay(MULTI_CLICK_TIMEOUT_MS);
      noInterrupts();
      clicks = stateButton.nPresses;
      interrupts();
    }
    stateButton.nPresses = 0;
  }
  
  Serial.print("Clicks: ");
  Serial.println(clicks);
  return clicks;
}