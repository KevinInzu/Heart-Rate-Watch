#include <Wire.h>
#include "MAX30105.h" // Sparkfun MAX3010X library
#include "SP02.h"
#include "Beat.h"

MAX30105 particleSensor;

#define TIMETOBOOT 3000 // wait for this time(msec) to output SpO2
#define SCALE 88.0      // adjust to display heart beat and SpO2 in the same scale
#define SAMPLING 5      // if you want to see heart beat more precisely, set SAMPLING to 1
#define FINGER_ON 30000 // if red signal is lower than this, it indicates your finger is not on the sensor

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
  if (irValue > 50000)
  {
    // Serial.print("Avg BPM: ");
    Serial.print(getBPM(irValue));
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