/*
  Health_Reader_Test_checkpoint2.ino
  04/20/23
  Undergraduate final project

  Main running code for the esp32 heartrate watch.

  Some sections of BLE code are based on library example by Neil Kolban
  created by Steve Graves
*/

///----LIBRARIES----///
#include <BLEDevice.h>
#include <BLEUtils.h>

#include <Wire.h>
#include "MAX30105.h"
#include "SP02.h"
#include "Beat.h"



MAX30105 particleSensor;

#define TIMETOBOOT 3000 // wait for this time(msec) to output SpO2
#define SCALE 88.0      // adjust to display heart beat and SpO2 in the same scale
#define SAMPLING 5      // if you want to see heart beat more precisely, set SAMPLING to 1
#define FINGER_ON 30000 // if red signal is lower than this, it indicates your finger is not on the sensor

float new_data;
float prev_data;






///----DEFINITIONS----///
// BLE Services
#define SERVICE_UUID_HR           "7da88714-c6c9-11ed-afa1-0242ac120002" //714
// BLE Characteristics (Write / Read)
#define CHARACTERISTIC_UUID_WRITE "7da889bc-c6c9-11ed-afa1-0242ac120002" //9BC
#define CHARACTERISTIC_UUID_READ  "7da88be2-c6c9-11ed-afa1-0242ac120002" //BE2

///----VARIABLES----///
BLEServer *pServer = NULL;
BLECharacteristic * pReadCharacteristic;
BLECharacteristic * pWriteCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
std::string readData[10];

int HeaderState = 0;

///----CALLBACKS----///
// Main Server Callback
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

// Characteristic Callbacks
class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string writeValue = pCharacteristic->getValue();

    if (writeValue.length() > 0) {
      Serial.print("Received Value: ");
        for (int i = 0; i < writeValue.length(); i++)
          Serial.print(writeValue[i]);

        Serial.println();
    }
  }
};


void setup() {
  // Device setup
  Serial.begin(115200);

  ///----BLE Setup----///
  BLEDevice::init("HeartWatchProto");
  
  /// Server creation and callback assignment
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID_HR);


  pReadCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_READ, BLECharacteristic::PROPERTY_READ);
  pWriteCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_WRITE, BLECharacteristic::PROPERTY_WRITE);

  pWriteCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();

  pServer->getAdvertising()->start();
  Serial.println("Waiting for a client to connect...");






  ///----HeartRate Setup----///
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


void connectCheck(){
  if (!deviceConnected && oldDeviceConnected) {
        Serial.print("Disconnected...\n");
        delay(1000); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
  }
    // connecting
  if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
        Serial.print("Connected!\n");
  }  
}
