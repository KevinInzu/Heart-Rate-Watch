#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include "Beat.h"

MAX30105 particleSensor;

byte ledBrightness = 0x7f;
byte sampleAverage = 4;
byte ledMode = 2;
int sampleRate = 200;
int pulseWidth = 411;
int adcRange = 16384;

int beats;

void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(ledBrightness,sampleAverage,ledMode,sampleRate, pulseWidth, adcRange); //Configure sensor with default settings
  // particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  // particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
}

void loop()
{
  long irValue = particleSensor.getIR();
  beatAvg =getBPM(irValue);

  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);

  if (irValue < 50000)
    Serial.print(" No finger?");

  Serial.println();
}


