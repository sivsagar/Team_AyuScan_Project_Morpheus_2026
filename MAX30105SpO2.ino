#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include "heartRate.h"

MAX30105 particleSensor;

uint32_t irBuffer[100];
uint32_t redBuffer[100];
int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

byte ledBrightness = 10;  // start very low
byte sampleAverage = 4;
byte ledMode = 2;
int sampleRate = 100;
int pulseWidth = 411;
int adcRange = 4096;

bool calibrated = false;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  Serial.println("AyuScan | MAX30102 Auto-Calibrating HR + SpO2");

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found. Check wiring!");
    while (1);
  }

  Serial.println("Sensor detected. Starting calibration...");
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
}

void loop() {
  if (!calibrated) {
    autoCalibrate();
  } else {
    measureVitals();
  }
}

// ───────────────────────────────
//  AUTO CALIBRATION FUNCTION
// ───────────────────────────────
void autoCalibrate() {
  long avgIR = 0;
  int count = 0;

  // read average IR level for current brightness
  for (int i = 0; i < 50; i++) {
    while (!particleSensor.available()) particleSensor.check();
    avgIR += particleSensor.getIR();
    count++;
    particleSensor.nextSample();
  }
  avgIR /= count;

  Serial.print("LED Brightness: "); Serial.print(ledBrightness);
  Serial.print("\tAverage IR: "); Serial.println(avgIR);

  if (avgIR < 80000 && ledBrightness < 200) {
    ledBrightness += 10;
    particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
    delay(300);
  } 
  else if (avgIR > 130000 && ledBrightness > 10) {
    ledBrightness -= 5;
    particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
    delay(300);
  } 
  else {
    calibrated = true;
    Serial.print("✅ Calibration complete. Optimal LED brightness = ");
    Serial.println(ledBrightness);
    Serial.println("Now measuring BPM + SpO2...");
  }
}

// ───────────────────────────────
//  MEASUREMENT FUNCTION
// ───────────────────────────────
void measureVitals() {
  const int bufferLength = 100;
  for (byte i = 0; i < bufferLength; i++) {
    while (!particleSensor.available()) particleSensor.check();
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i]  = particleSensor.getIR();
    particleSensor.nextSample();
  }

  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength,
                                         redBuffer, &spo2, &validSPO2,
                                         &heartRate, &validHeartRate);

  Serial.println("────────────────────────────");
  Serial.print("Heart Rate (BPM): ");
  if (validHeartRate) Serial.println(heartRate);
  else Serial.println("Invalid");

  Serial.print("SpO2 (%): ");
  if (validSPO2) Serial.println(spo2);
  else Serial.println("Invalid");

  Serial.println("────────────────────────────");
  delay(1000);
}
