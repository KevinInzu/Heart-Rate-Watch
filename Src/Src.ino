#include <Wire.h>
#include <deque>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include "MAX30105.h" // Sparkfun MAX3010X library
#include "SP02.h"
#include "Beat.h"
#include "ScrollingWindow.h"

MAX30105 particleSensor;

#define FINGER_ON 50000 // if red signal is lower than this, it indicates your finger is not on the sensor

int new_data;
int prev_data;

deque<int> window1;
int window_size1 = 4;
int threshold1 = 6;

vector<int> bpm;

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

  if (irValue > FINGER_ON && getBPM(irValue) != 0 && new_data != prev_data && new_data < 400)
  {
    Serial.print("Avg BPM: ");
    Serial.println(new_data);
    // Serial.print(" , ");

    bpm.push_back(new_data);
    maintain_window(bpm,new_data, window1, window_size1, threshold1);
    // // Serial.print("SP02: ");
    // Serial.print(getSPO2(particleSensor));
    // Serial.print(" , ");

    // particleSensor.enableDIETEMPRDY();
    // float temperatureF = particleSensor.readTemperatureF();
    // // Serial.print("TemperatureF: ");
    // Serial.println(temperatureF, 4);
  }
  else if ( irValue < FINGER_ON) //TODO: Change when user prompts another reading
  {
    Serial.println("New BPM values: ");
    for (int nums : bpm)
    {
      Serial.print(nums);
      Serial.print(" , ");
    }
    Serial.println();
    bpm.clear();
    window1.clear();
  }
}