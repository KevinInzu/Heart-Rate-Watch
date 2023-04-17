#include <Wire.h>
#include "MAX30105.h" // Sparkfun MAX3010X library
#include "SP02.h"

MAX30105 particleSensor;


#define TIMETOBOOT 3000 // wait for this time(msec) to output SpO2
#define SCALE 88.0 // adjust to display heart beat and SpO2 in the same scale
#define SAMPLING 5 // if you want to see heart beat more precisely, set SAMPLING to 1
#define FINGER_ON 30000 // if red signal is lower than this, it indicates your finger is not on the sensor

void setup() {
  Serial.begin(115200);

  // Initialize sensor
  while (!particleSensor.begin(Wire, I2C_SPEED_FAST)) { // Use default I2C port, 400kHz speed
    Serial.println("MAX30102 was not found. Please check wiring/power/solder jumper at MH-ET LIVE MAX30102 board. ");
  }

  // Setup to sense a nice looking sawtooth on the plotter
  byte ledBrightness = 0x7F; // Options: 0=Off to 255=50mA
  byte sampleAverage = 4; // Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; // Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  // Options: 1 = IR only, 2 = Red + IR on MH-ET LIVE MAX30102 board
  int sampleRate = 200; // Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; // Options: 69, 118, 215, 411
  int adcRange = 16384; // Options: 2048, 4096, 8192, 16384
  // Set up the wanted parameters
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); // Configure sensor with these settings

}
void loop()
{
  Serial.println(getSPO2(particleSensor));
}