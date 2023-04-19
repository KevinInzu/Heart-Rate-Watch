#include <Wire.h>
#include "MAX30105.h" // Sparkfun MAX3010X library
#include "SP02.h"
#include "Beat.h"

MAX30105 particleSensor;

#define TIMETOBOOT 3000 // wait for this time(msec) to output SpO2
#define SCALE 88.0      // adjust to display heart beat and SpO2 in the same scale
#define SAMPLING 5      // if you want to see heart beat more precisely, set SAMPLING to 1
#define FINGER_ON 30000 // if red signal is lower than this, it indicates your finger is not on the sensor

float new_data;
float prev_data;

void setup()
{
  Serial.begin(115200);

  // Initialize sensor
  while (!particleSensor.begin(Wire, I2C_SPEED_FAST))
  { // Use default I2C port, 400kHz speed
    Serial.println("MAX30102 was not found. Please check wiring/power/solder jumper at MH-ET LIVE MAX30102 board. ");
  }

  byte ledBrightness = 0x7F;
  byte sampleAverage = 4;
  byte ledMode = 2;

  int sampleRate = 200;
  int pulseWidth = 411;
  int adcRange = 16384;

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
}
void loop()
{
  // delay(60000);
  long irValue = particleSensor.getIR();

  prev_data = new_data;
  new_data = getBPM(irValue);


  if (irValue > 50000 && getBPM(irValue) != 0 && new_data != prev_data)
  {
    // Serial.print("Avg BPM: ");
    Serial.print(new_data);
    Serial.print(" , ");

    // Serial.print("SP02: ");
    Serial.print(getSPO2(particleSensor));
    Serial.print(" , ");

    particleSensor.enableDIETEMPRDY();
    float temperatureF = particleSensor.readTemperatureF();
    // Serial.print("TemperatureF: ");
    Serial.println(temperatureF, 4);
  }
}