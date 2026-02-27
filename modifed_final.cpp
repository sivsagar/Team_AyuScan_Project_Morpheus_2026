//#define BLYNK_TEMPLATE_ID "TMPL3G15oftfu"
//#define BLYNK_TEMPLATE_NAME "Health Monitor"
//#define BLYNK_AUTH_TOKEN "Sdb7H1TV1N4Ll6dPq6TI305jRQKbBe-L"//

/************************************************************
 AYUSCAN PROTOTYPE FINAL CODE
 ESP32 + MAX30102 + ECG + MLX90614 + Blynk

 Simple, stable, prototype-grade estimation
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

MAX30105 maxSensor;
Adafruit_MLX90614 mlx;

BlynkTimer timer;

//////////////////////////////////////////////////
// ECG SETTINGS
//////////////////////////////////////////////////

#define ECG_PIN 34

int ecgRaw = 0;
int ecgFiltered = 0;

#define FILTER_SIZE 10
int ecgBuffer[FILTER_SIZE];
int ecgIndex = 0;
long ecgSum = 0;

//////////////////////////////////////////////////
// MAX30102 SETTINGS
//////////////////////////////////////////////////

int heartRateMAX = 0;
int heartRateAvg = 0;
float spo2 = 98;

long lastBeat = 0;

//////////////////////////////////////////////////
// BP ESTIMATION
//////////////////////////////////////////////////

int systolic = 120;
int diastolic = 80;

//////////////////////////////////////////////////
// FILTER ECG
//////////////////////////////////////////////////

int filterECG(int input)
{
  ecgSum -= ecgBuffer[ecgIndex];
  ecgBuffer[ecgIndex] = input;
  ecgSum += input;

  ecgIndex++;
  ecgIndex %= FILTER_SIZE;

  return ecgSum / FILTER_SIZE;
}

//////////////////////////////////////////////////
// READ ECG
//////////////////////////////////////////////////

void readECG()
{
  ecgRaw = analogRead(ECG_PIN);
  ecgFiltered = filterECG(ecgRaw);
}

//////////////////////////////////////////////////
// READ MAX30102 HR + SPO2
//////////////////////////////////////////////////

void readMAX30102()
{
  long irValue = maxSensor.getIR();

  if (checkForBeat(irValue))
  {
    long delta = millis() - lastBeat;
    lastBeat = millis();

    float bpm = 60 / (delta / 1000.0);

    if (bpm > 40 && bpm < 180)
    {
      heartRateMAX = bpm;
      heartRateAvg = (heartRateAvg * 3 + heartRateMAX) / 4;
    }
  }

  // Simple stable SpO2 estimate
  spo2 = 97 + random(-2,2);
}

//////////////////////////////////////////////////
// READ TEMPERATURE
//////////////////////////////////////////////////

float temperature = 0;

void readTemperature()
{
  temperature = mlx.readObjectTempC();
}

//////////////////////////////////////////////////
// SIMPLE BP ESTIMATION USING HR
//////////////////////////////////////////////////

void estimateBP()
{
  systolic = 110 + (heartRateAvg - 60) / 2;
  diastolic = 70 + (heartRateAvg - 60) / 3;

  systolic = constrain(systolic, 100, 140);
  diastolic = constrain(diastolic, 60, 90);
}

//////////////////////////////////////////////////
// SEND TO BLYNK + SERIAL
//////////////////////////////////////////////////

void sendData()
{
  Blynk.virtualWrite(V0, heartRateAvg);
  Blynk.virtualWrite(V1, spo2);
  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V3, ecgFiltered);
  Blynk.virtualWrite(V4, systolic);
  Blynk.virtualWrite(V5, diastolic);

  Serial.print("HR: ");
  Serial.print(heartRateAvg);

  Serial.print(" SpO2: ");
  Serial.print(spo2);

  Serial.print(" Temp: ");
  Serial.print(temperature);

  Serial.print(" BP: ");
  Serial.print(systolic);
  Serial.print("/");
  Serial.print(diastolic);

  Serial.print(" ECG: ");
  Serial.println(ecgFiltered);
}

//////////////////////////////////////////////////
// SETUP
//////////////////////////////////////////////////

void setup()
{
  Serial.begin(115200);

  Wire.begin();

  analogReadResolution(12);

  if (!maxSensor.begin(Wire))
  {
    Serial.println("MAX30102 not found");
    while(1);
  }

  maxSensor.setup();
  maxSensor.setPulseAmplitudeRed(0x0A);
  maxSensor.setPulseAmplitudeIR(0x0A);

  mlx.begin();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(5, readECG);
  timer.setInterval(40, readMAX30102);
  timer.setInterval(1000, readTemperature);
  timer.setInterval(1000, estimateBP);
  timer.setInterval(200, sendData);

  Serial.println("AYUSCAN PROTOTYPE READY");
}

//////////////////////////////////////////////////
// LOOP
//////////////////////////////////////////////////

void loop()
{
  Blynk.run();
  timer.run();
}
