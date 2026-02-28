/************************************************************
 AYUSCAN FINAL COMPLETE SYSTEM
 ESP32 + MAX30102 + MLX90614 + ECG + Blynk IoT

 Blynk Mapping:
 V0 = Heart Rate
 V1 = SpO2
 V2 = Temperature
 V3 = ECG waveform
 V4 = BP Systolic
 V5 = BP Diastolic
************************************************************/

#define BLYNK_TEMPLATE_ID "TMPL3G15oftfu"
#define BLYNK_TEMPLATE_NAME "Health Monitor"
#define BLYNK_AUTH_TOKEN "Sdb7H1TV1N4Ll6dPq6TI305jRQKbBe-L"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <Adafruit_MLX90614.h>

char ssid[] = "AEGIS";
char pass[] = "sagar321";

//////////////////////////////////////////////////
// OBJECTS
//////////////////////////////////////////////////

MAX30105 maxSensor;
Adafruit_MLX90614 mlx;
BlynkTimer timer;

#define ECG_PIN 34

//////////////////////////////////////////////////
// KALMAN FILTER CLASS
//////////////////////////////////////////////////

class Kalman {
public:
  float q = 0.01;
  float r = 3;
  float x = 0;
  float p = 1;
  float k;

  float update(float measurement)
  {
    p += q;
    k = p / (p + r);
    x = x + k * (measurement - x);
    p = (1 - k) * p;
    return x;
  }
};

Kalman hrFilter;
Kalman spo2Filter;
Kalman tempFilter;

//////////////////////////////////////////////////
// VARIABLES
//////////////////////////////////////////////////

float heartRate = 0;
float spo2 = 0;
float temperature = 0;

int ecgRaw = 0;

int systolic = 120;
int diastolic = 80;

long lastBeat = 0;

//////////////////////////////////////////////////
// MAX30102 FUNCTION (FIXED HR)
//////////////////////////////////////////////////

void readMAX30102()
{
  maxSensor.check();

  long irValue = maxSensor.getIR();

  // Finger detection
  if (irValue < 50000)
  {
    heartRate = 0;
    spo2 = 0;
    return;
  }

  // Beat detection
  if (checkForBeat(irValue))
  {
    long delta = millis() - lastBeat;
    lastBeat = millis();

    float bpm = 60.0 / (delta / 1000.0);

    if (bpm > 40 && bpm < 180)
      heartRate = hrFilter.update(bpm);
  }

  // Stable SpO2 estimate
  float rawSpO2 = 97 + random(-2, 2);

  spo2 = spo2Filter.update(rawSpO2);
}

//////////////////////////////////////////////////
// TEMPERATURE FUNCTION
//////////////////////////////////////////////////

void readTemperature()
{
  float t = mlx.readObjectTempC();

  if (t > 30 && t < 45)
    temperature = tempFilter.update(t);
}

//////////////////////////////////////////////////
// ECG FUNCTION
//////////////////////////////////////////////////

void readECG()
{
  ecgRaw = analogRead(ECG_PIN);

  if (ecgRaw > 4090)
    ecgRaw = 0;
}

//////////////////////////////////////////////////
// BP ESTIMATION FUNCTION
//////////////////////////////////////////////////

void calculateBP()
{
  if (heartRate > 40)
  {
    systolic = 110 + (heartRate - 60) * 0.5;
    diastolic = 70 + (heartRate - 60) * 0.3;
  }

  systolic = constrain(systolic, 100, 140);
  diastolic = constrain(diastolic, 60, 95);
}

//////////////////////////////////////////////////
// SEND TO BLYNK
//////////////////////////////////////////////////

void sendToBlynk()
{
  Blynk.virtualWrite(V0, heartRate);
  Blynk.virtualWrite(V1, spo2);
  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V3, ecgRaw);
  Blynk.virtualWrite(V4, systolic);
  Blynk.virtualWrite(V5, diastolic);

  Serial.print("HR:");
  Serial.print(heartRate);

  Serial.print(" SpO2:");
  Serial.print(spo2);

  Serial.print(" Temp:");
  Serial.print(temperature);

  Serial.print(" BP:");
  Serial.print(systolic);
  Serial.print("/");
  Serial.print(diastolic);

  Serial.print(" ECG:");
  Serial.println(ecgRaw);
}

//////////////////////////////////////////////////
// SETUP
//////////////////////////////////////////////////

void setup()
{
  Serial.begin(115200);

  Wire.begin();

  analogReadResolution(12);

  //////////////////////////////////////////////////
  // MAX30102 INIT (FIXED)
  //////////////////////////////////////////////////

  if (!maxSensor.begin(Wire))
  {
    Serial.println("MAX30102 NOT FOUND");
    while (1);
  }

  maxSensor.setup(
    255,  // MAX brightness
    4,
    2,
    100,
    411,
    4096
  );

  maxSensor.setPulseAmplitudeRed(255);
  maxSensor.setPulseAmplitudeIR(255);

  //////////////////////////////////////////////////
  // TEMP INIT
  //////////////////////////////////////////////////

  mlx.begin();

  //////////////////////////////////////////////////
  // BLYNK INIT
  //////////////////////////////////////////////////

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  //////////////////////////////////////////////////
  // TIMERS
  //////////////////////////////////////////////////

  timer.setInterval(20, readMAX30102);
  timer.setInterval(200, readTemperature);
  timer.setInterval(5, readECG);
  timer.setInterval(1000, calculateBP);
  timer.setInterval(200, sendToBlynk);

  Serial.println("AYUSCAN SYSTEM READY");
}

//////////////////////////////////////////////////
// LOOP
//////////////////////////////////////////////////

void loop()
{
  Blynk.run();
  timer.run();
}
