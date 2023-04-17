#include <Wire.h>
#include "MAX30105.h" // Sparkfun MAX3010X library
double avered = 0;
double aveir = 0;
double sumirrms = 0;
double sumredrms = 0;
int i = 0;
int Num = 100;       // calculate SpO2 by this sampling interval
float ESpO2;         // initial value of estimated SpO2
double FSpO2 = 0.7;  // filter factor for estimated SpO2
double frate = 0.95; // low pass filter for IR/red LED value to eliminate AC component

float getSPO2(MAX30105 &particleSensor)
{

    uint32_t ir, red;
    double fred, fir;
    double SpO2 = 0; // raw SpO2 before low pass filtered

    particleSensor.check(); // Check the sensor, read up to 3 samples

    while (particleSensor.available())
    { // do we have new data

        red = particleSensor.getFIFOIR(); // why getFOFOIR output Red data by MAX30102 on MH-ET LIVE breakout board
        ir = particleSensor.getFIFORed(); // why getFIFORed output IR data by MAX30102 on MH-ET LIVE breakout board
        // Serial.println(ir);

        
        i++;
        fred = (double)red;
        fir = (double)ir;
        avered = avered * frate + (double)red * (1.0 - frate); // average red level by low pass filter
        aveir = aveir * frate + (double)ir * (1.0 - frate);    // average IR level by low pass filter
        sumredrms += (fred - avered) * (fred - avered);        // square sum of alternate component of red level
        sumirrms += (fir - aveir) * (fir - aveir);             // square sum of alternate component of IR level

        if ((i % Num) == 0)
        {
            double R = (sqrt(sumredrms) / avered) / (sqrt(sumirrms) / aveir);
            SpO2 = -23.3 * (R - 0.4) + 100;               // http://ww1.microchip.com/downloads/jp/AppNotes/00001525B_JP.pdf
            ESpO2 = FSpO2 * ESpO2 + (1.0 - FSpO2) * SpO2; // low pass filter
            sumredrms = 0.0;
            sumirrms = 0.0;
            i = 0;
            break;
        }
        particleSensor.nextSample(); // We're finished with this sample so move
    }
    return ESpO2;
}