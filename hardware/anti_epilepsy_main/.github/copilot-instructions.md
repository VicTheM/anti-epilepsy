# Anti-Epilepsy Device - AI Coding Guidelines

## Project Overview
This is an ESP32-based anti-epilepsy monitoring and intervention device that combines multiple sensors to detect seizures and automatically administer medication via microneedle injection.

## Architecture & Key Components

### Hardware Integration Pattern
- **MPU6050**: 6-axis accelerometer/gyroscope for motion detection (`Adafruit_MPU6050` library)
- **MAX30105**: Heart rate/pulse oximetry sensor for vital signs monitoring
- **Servo Motor**: Controls microneedle drug delivery mechanism (`ESP32Servo` library)
- **Output Devices**: Pump (GPIO 25), Buzzer (GPIO 17), LED indicator (GPIO 18)

### Code Structure
- `anti_epilepsy_main.ino`: Setup and main loop (currently minimal loop)
- `anti_epilepsy.h`: Pin definitions, thresholds, structs, and function prototypes
- `anti_epilepsy.cpp`: Sensor initialization, data reading, and control functions

## Critical Development Patterns

### Sensor Data Handling
All sensor data uses dedicated structs for clean data passing:
```cpp
MPU6050Data mpuData;  // Motion data (ax,ay,az,gx,gy,gz,temp)
MAX30105Data heartData;  // Heart rate data (irValue,bpm,avgBpm,fingerDetected)
```

### Initialization Pattern
Each sensor follows a consistent init/read pattern:
```cpp
if (!initMAX30105(particleSensor)) {
    Serial.println("Error initializing heart beat rate sensor");
    while (n++ != 20) delay(10);  // Retry mechanism
}
```

### Seizure Detection Logic
Located in `checkForSeizure()` - combines heart rate anomalies with excessive motion:
- BPM thresholds: 60-120 (configurable via `#define`)
- Acceleration threshold: 15.0 m/s²
- Gyroscope threshold: 3.0 rad/s
- Requires finger detection (`irValue > NO_FINGER_THRESHOLD`)

### Servo Control Precision
`moveServo()` implements smooth movement with speed control and automatic return to position - critical for safe drug delivery.

## Development Workflow

### Pin Configuration
All hardware pins defined in header constants (`PUMP_PIN`, `SERVO_PIN`, etc.) - modify these for different ESP32 boards.

### Serial Debugging
Project uses 115200 baud rate. All major functions include Serial output for debugging hardware interactions.

### Sensor Calibration
- MPU6050 configured for ±8G acceleration, ±500°/s gyroscope, 21Hz filter
- MAX30105 uses default setup with red LED amplitude 0x0A, green LED off

## Key Implementation Notes

### Error Handling
Hardware initialization failures use retry loops with delay, not exceptions. Check return values from all `init*()` functions.

### Real-time Constraints
Main loop is currently empty - seizure detection should run continuously. Consider implementing state machine for device modes.

### Safety Considerations
Drug administration (`administerDrug()`) function is declared but not implemented - requires careful safety validation before implementation.

### Missing Components
- Main loop seizure monitoring logic
- Drug administration implementation
- Data logging/storage
- Communication protocols (WiFi/Bluetooth)

## When Modifying This Code
1. Always test sensor initialization on hardware before implementing detection logic
2. Validate threshold values against real physiological data
3. Implement proper state management for device operational modes
4. Add bounds checking for all servo movements and pump operations
5. Consider power management for battery-operated deployment