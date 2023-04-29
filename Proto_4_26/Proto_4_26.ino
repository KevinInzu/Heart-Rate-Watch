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
#include <deque>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <Wire.h>
#include "MAX30105.h"
#include "SP02.h"
#include "Beat.h"
#include "ScrollingWindow.h"

MAX30105 particleSensor;

#define TIMETOBOOT 3000 // wait for this time(msec) to output SpO2
#define SCALE 88.0      // adjust to display heart beat and SpO2 in the same scale
#define SAMPLING 5      // if you want to see heart beat more precisely, set SAMPLING to 1
#define FINGER_ON 50000 // if red signal is lower than this, it indicates your finger is not on the sensor

float new_data;
float prev_data;

deque<int> window1;
int window_size1 = 4;
int threshold1 = 6;

vector<float> bpm;




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
//std::string[] readData = new
std::vector<float> readData = {};

std::string writeValue;

int HeaderState = 0;
int DataCount = 0; 


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
    writeValue = pCharacteristic->getValue();

    if (writeValue.length() > 0) {
        for (int i = 0; i < writeValue.length()-1; i++)
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




  byte ledBrightness = 0x7F;
  byte sampleAverage = 4;
  byte ledMode = 2;

  int sampleRate = 200;
  int pulseWidth = 411;
  int adcRange = 16384;


  particleSensor.begin(Wire, I2C_SPEED_FAST); // Must be I2C FAST OR THERE IS A TIMING ISSUE
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
  particleSensor.shutDown();
}


void HeartRateCollect(){
   // delay(60000);
  long irValue = particleSensor.getIR();

  prev_data = new_data;
  new_data = getBPM(irValue);

  if (irValue > FINGER_ON && getBPM(irValue) != 0 && new_data != prev_data && new_data < 400)
  {
    Serial.print("Avg BPM: ");
    Serial.println(new_data);
    // Serial.print(" , ");

    // bpm.push_back(new_data);
    maintain_window(new_data, window1, window_size1, threshold1);
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
    window1.clear();
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



void loop()
{
  if(HeaderState == 0){ //BOOT STATE - verify connection, wait for start request
    connectCheck();
    if(writeValue[0] == 'B'){
      Serial.print("B Recieved: Entering State-1");
      particleSensor.wakeUp();
      HeaderState = 1;
    }
  }
  else if(HeaderState == 1){ //NEXT STATE - start heart rate reading
    HeartRateCollect();
  }
  else if(HeaderState == 2){
    for(int i=0; i < window1.size(); i++){
      Serial.print(window1.at(i));;
    }
    window1.clear();
    HeaderState = 0;
  }
}
