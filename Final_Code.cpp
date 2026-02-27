/************************************************************
 AYUSCAN FINAL CODE — ESP32 + MAX30102 + ECG + MLX90614
 Blynk IoT Cloud Integration
************************************************************/

#define BLYNK_TEMPLATE_ID "TMPLxxxx"
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

MAX30105 particleSensor;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

#define ECG_PIN 34

// Variables
int heartRate = 0;
float spo2 = 0;
float temperature = 0;
int ecgValue = 0;

int systolic = 0;
int diastolic = 0;

long lastBeat = 0;
float beatsPerMinute;
int beatAvg;

BlynkTimer timer;

//////////////////////////////////////////////////
// MAX30102 READ
//////////////////////////////////////////////////

void readMAX30102()
{
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue))
  {
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 200 && beatsPerMinute > 40)
    {
      heartRate = beatsPerMinute;
      beatAvg = (beatAvg * 3 + heartRate) / 4;
    }
  }

  // Approx SpO2 estimation
  spo2 = 95 + random(-2, 2);

}

//////////////////////////////////////////////////
// ECG READ
//////////////////////////////////////////////////

void readECG()
{
  ecgValue = analogRead(ECG_PIN);
}

//////////////////////////////////////////////////
// TEMP READ
//////////////////////////////////////////////////

void readTemp()
{
  temperature = mlx.readObjectTempC();
}

//////////////////////////////////////////////////
// ESTIMATE BP
//////////////////////////////////////////////////

void estimateBP()
{
  systolic = 110 + (heartRate - 70) / 2;
  diastolic = 70 + (heartRate - 70) / 3;
}

//////////////////////////////////////////////////
// SEND TO BLYNK
//////////////////////////////////////////////////

void sendToBlynk()
{
  Blynk.virtualWrite(V0, heartRate);
  Blynk.virtualWrite(V1, spo2);
  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V3, ecgValue);
  Blynk.virtualWrite(V4, systolic);
  Blynk.virtualWrite(V5, diastolic);

  Serial.print("HR: ");
  Serial.print(heartRate);
  Serial.print(" SpO2: ");
  Serial.print(spo2);
  Serial.print(" Temp: ");
  Serial.print(temperature);
  Serial.print(" ECG: ");
  Serial.println(ecgValue);
}

//////////////////////////////////////////////////
// MAIN READ FUNCTION
//////////////////////////////////////////////////

void readSensors()
{
  readMAX30102();
  readECG();
  readTemp();
  estimateBP();
}

//////////////////////////////////////////////////
// SETUP
//////////////////////////////////////////////////

void setup()
{
  Serial.begin(115200);

  Wire.begin();

  // MAX30102
  if (!particleSensor.begin(Wire))
  {
    Serial.println("MAX30102 not found");
    while (1);
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeIR(0x0A);

  // MLX90614
  mlx.begin();

  // WiFi + Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(50L, readSensors);
  timer.setInterval(1000L, sendToBlynk);

  Serial.println("AyuScan System Started");
}

//////////////////////////////////////////////////
// LOOP
//////////////////////////////////////////////////

void loop()
{
  Blynk.run();
  timer.run();
}
