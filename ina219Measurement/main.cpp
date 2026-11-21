#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

// INA219 instance
Adafruit_INA219 ina219;

// Timing control for 10Hz sampling
unsigned long lastMeasurement = 0;
const unsigned long SAMPLE_INTERVAL = 100; // 100ms = 10Hz

// Statistical analysis variables
const int MOVING_AVG_SIZE = 10;
float currentBuffer[MOVING_AVG_SIZE];
int bufferIndex = 0;
bool bufferFilled = false;

// Measurement statistics
float currentSum = 0;
float currentAvg = 0;
unsigned long measurementCount = 0;

// Noise filtering
const int OUTLIER_THRESHOLD = 50; // μA threshold for outlier detection
float lastValidCurrent = 0;

// Timing
unsigned long startTime = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  
  // Initialize I2C and INA219
  Wire.begin();
  
  if (!ina219.begin()) {
    Serial.println("ERROR: INA219 not found!");
    while (1) delay(1000);
  }
  
  // Configure for maximum precision
  ina219.setCalibration_32V_1A();
  
  // Initialize measurement arrays
  for (int i = 0; i < MOVING_AVG_SIZE; i++) {
    currentBuffer[i] = 0;
  }
  
  startTime = millis();
  
  // Print CSV header (comment out for pure data)
  Serial.println("Time_ms,Bus_V,Shunt_mV,Current_mA,Power_mW,Current_uA,Avg_uA");
  
  delay(1000); // Allow system to stabilize
}

void loop() {
  unsigned long currentTime = millis();
  
  // Check if it's time for next measurement (10Hz sampling)
  if (currentTime - lastMeasurement >= SAMPLE_INTERVAL) {
    lastMeasurement = currentTime;
    
    // Take measurements
    float busVoltage = ina219.getBusVoltage_V();
    float shuntVoltage = ina219.getShuntVoltage_mV();
    float current_mA = ina219.getCurrent_mA();
    float power_mW = ina219.getPower_mW();
    
    // Convert to μA for high precision display
    float current_uA = current_mA * 1000.0;
    
    // Simple noise filtering
    if (measurementCount > 0) {
      float currentDiff = abs(current_uA - lastValidCurrent);
      if (currentDiff > OUTLIER_THRESHOLD && measurementCount > 5) {
        current_uA = lastValidCurrent; // Use previous valid measurement
      }
    }
    lastValidCurrent = current_uA;
    
    // Update moving average buffer
    currentBuffer[bufferIndex] = current_uA;
    bufferIndex = (bufferIndex + 1) % MOVING_AVG_SIZE;
    if (bufferIndex == 0) bufferFilled = true;
    
    // Calculate moving average
    currentSum = 0;
    int samplesInAvg = bufferFilled ? MOVING_AVG_SIZE : bufferIndex;
    for (int i = 0; i < samplesInAvg; i++) {
      currentSum += currentBuffer[i];
    }
    currentAvg = currentSum / samplesInAvg;
    
    measurementCount++;
    
    // Output CSV format: Time,Bus_V,Shunt_mV,Current_mA,Power_mW,Current_uA,Avg_uA
    unsigned long relativeTime = currentTime - startTime;
    
    Serial.print(relativeTime);
    Serial.print(",");
    Serial.print(busVoltage, 3);
    Serial.print(",");
    Serial.print(shuntVoltage, 2);
    Serial.print(",");
    Serial.print(current_mA, 3);
    Serial.print(",");
    Serial.print(power_mW, 2);
    Serial.print(",");
    Serial.print(current_uA, 1);
    Serial.print(",");
    Serial.print(currentAvg, 1);
    Serial.println();
  }
  
  // Simple command processing (optional)
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command == "reset") {
      measurementCount = 0;
      bufferIndex = 0;
      bufferFilled = false;
      startTime = millis();
      // Print header again after reset
      Serial.println("Time_ms,Bus_V,Shunt_mV,Current_mA,Power_mW,Current_uA,Avg_uA");
    }
  }
}