#include <Wire.h>
#include "MAX30105.h"

MAX30105 sensor;

#define SDA_PIN 22
#define SCL_PIN 23

byte ledPower = 10;

long airIR = 0;
long wristIR = 0;

void setup()
{
  Serial.begin(115200);
  delay(2000);

  Wire.begin(SDA_PIN, SCL_PIN);

  Serial.println("\nAyuScan MAX30102 Calibration");

  if (!sensor.begin(Wire))
  {
    Serial.println("MAX30102 not found");
    while (1);
  }

  sensor.setup(
    ledPower,
    4,
    2,
    100,
    411,
    4096
  );

  Serial.println("\nSTEP 1: Keep sensor in AIR");
  delay(5000);

  airIR = readAverageIR();

  Serial.print("Air IR = ");
  Serial.println(airIR);

  Serial.println("\nSTEP 2: Place sensor on WRIST");
  delay(5000);

  autoAdjustLED();

  wristIR = readAverageIR();

  Serial.print("Wrist IR = ");
  Serial.println(wristIR);

  int threshold = wristIR * 0.4;

  Serial.println("\nCALIBRATION RESULTS:");
  Serial.print("#define MAX_LED_POWER ");
  Serial.println(ledPower);

  Serial.print("#define MAX_IR_THRESHOLD ");
  Serial.println(threshold);

  Serial.println("\nUse these values in main firmware.");
}

void loop()
{
}

long readAverageIR()
{
  long sum = 0;

  for (int i = 0; i < 50; i++)
  {
    sensor.check();

    while (!sensor.available())
      sensor.check();

    sum += sensor.getIR();
    sensor.nextSample();
  }

  return sum / 50;
}

void autoAdjustLED()
{
  Serial.println("\nAuto adjusting LED...");

  while (true)
  {
    sensor.setPulseAmplitudeIR(ledPower);
    sensor.setPulseAmplitudeRed(ledPower);

    delay(500);

    long avgIR = readAverageIR();

    Serial.print("LED: ");
    Serial.print(ledPower);
    Serial.print(" IR: ");
    Serial.println(avgIR);

    if (avgIR < 60000 && ledPower < 150)
      ledPower += 5;
    else if (avgIR > 120000 && ledPower > 10)
      ledPower -= 5;
    else
      break;
  }

  Serial.println("LED calibrated");
}
