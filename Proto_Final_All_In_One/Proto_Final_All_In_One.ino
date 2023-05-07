/*
  Proto_Final_All_In_One.ino
  05/05/23
  Undergraduate final project (CSE 5408)

  created by Steve Graves & Kevin Inzunza
*/

///   ----LIBRARIES----   ///
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <Wire.h>
#include <sstream>
#include <deque>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <iterator>
#include "MAX30105.h"
#include "SP02.h"
#include "Beat.h"
#include "ScrollingWindow.h"



///   ----DEFINITIONS----   ///
#define FINGER_ON 50000 // if red signal is lower than this, it indicates your finger is not on the sensor
#define SERVICE_UUID_HR           "7da88714-c6c9-11ed-afa1-0242ac120002" // BLE Services
#define CHARACTERISTIC_UUID_WRITE "7da889bc-c6c9-11ed-afa1-0242ac120002" // BLE Characteristics (Write)
#define CHARACTERISTIC_UUID_READ  "7da88be2-c6c9-11ed-afa1-0242ac120002" // BLE Characteristics (Read)



///   ----VARIABLES----   ///
MAX30105 particleSensor;

int new_data;
int prev_data;
int window_size1 = 4;
int threshold1 = 6;
int HeaderState = 0;
int sampleRate = 200;
int pulseWidth = 411;
int adcRange = 16384;

byte ledBrightness = 0x7F;
byte sampleAverage = 4;
byte ledMode = 2;

deque<int> window1;
vector<int> bpm;
vector<int> spo;
vector<int> temp;


BLEServer *pServer = NULL;
BLECharacteristic * pReadCharacteristic;
BLECharacteristic * pWriteCharacteristic;

bool deviceConnected = false;
bool oldDeviceConnected = false;

stringstream result, result2, result3;
std::string readstring;
std::string writeValue;



///   ----CALLBACKS----   ///
class MyServerCallbacks: public BLEServerCallbacks {  // Main Server Callback
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

class MyCallbacks: public BLECharacteristicCallbacks {  // Characteristic Callbacks - Write
  void onWrite(BLECharacteristic *pCharacteristic) {
    writeValue = pCharacteristic->getValue();

    if (writeValue.length() > 0) {
        for (int i = 0; i < writeValue.length()-1; i++)
          Serial.print(writeValue[i]);

        Serial.println();
    }
  }
};



///   ----SETUP FUNCTION----   ///
void setup() {
  Serial.begin(115200);  // Device setup

  BLEDevice::init("HeartWatchProto"); // BLE setup
  
  pServer = BLEDevice::createServer();  // Server/service creation and callback assignment
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID_HR);

  pReadCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_READ, BLECharacteristic::PROPERTY_READ); // Read characteristic Property assignment
  pWriteCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_WRITE, BLECharacteristic::PROPERTY_WRITE); // Write characteristic Property assignment

  pWriteCharacteristic->setCallbacks(new MyCallbacks());  // Write Characteristic callback assignment

  pService->start(); // Start BLE service

  pServer->getAdvertising()->start(); // Allow Server to read advertising packets
  Serial.println("Waiting for a client to connect...");


  particleSensor.begin(Wire, I2C_SPEED_FAST); // Must be I2C FAST OR THERE IS A TIMING ISSUE
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
  particleSensor.shutDown();

}



///   ----CUSTOM FUNCTIONS----   ///
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


void HeartRateCollect(){
  long irValue = particleSensor.getIR();
  long SpValue = getSPO2(particleSensor);

  prev_data = new_data;
  new_data = getBPM(irValue);

  if (irValue > FINGER_ON && getBPM(irValue) != 0 && new_data < 400 && new_data != prev_data)
  {
    Serial.print("Avg BPM: ");
    Serial.println(new_data);

    bpm.push_back(new_data);
    maintain_window(bpm,new_data, window1, window_size1, threshold1);

    Serial.print("SP02: ");
    Serial.print(getSPO2(particleSensor));

    spo.push_back(SpValue);
  }
  else{
    connectCheck();
    if(writeValue[0] == 'B'){
      HeaderState = 3;
    }
  }
}

void TemperatureCollect(){
  long irValue = particleSensor.getIR();
  if (irValue > FINGER_ON){
   particleSensor.enableDIETEMPRDY();
   float temperatureF = particleSensor.readTemperatureF();
   Serial.print("TemperatureF: ");
   Serial.println(temperatureF, 4);
   temp.push_back(temperatureF);
  }
  else{
   connectCheck();
   if(writeValue[0] == 'C'){
      HeaderState = 4;
      window1.clear();
      particleSensor.shutDown();
    }
  }
}


void loop() {
  delay(10);
  if(HeaderState == 0){ //BOOT STATE - verify connection, wait for start request
    connectCheck();
    if(writeValue[0] == 'A'){
      Serial.println("Recieved: Entering State-1");
      HeaderState = 1;
    }
  }
  else if(HeaderState == 1){ //NEXT STATE - start heart rate reading
    Serial.println("Waking particleSensor");
    particleSensor.wakeUp();
    HeaderState = 2;
  }
  else if(HeaderState == 2){
    HeartRateCollect();
  }
  else if(HeaderState == 3){
    TemperatureCollect();
  }
  else if(HeaderState == 4){
    copy(bpm.begin(), bpm.end(), ostream_iterator<int>(result, " "));
    readstring = result.str();
    readstring = readstring.substr(0, readstring.length()-1);
    pReadCharacteristic->setValue(readstring);
    bpm.clear();
    HeaderState = 5;
  }
  else if(HeaderState == 5){
    connectCheck();
    if(writeValue[0] == 'D'){
      copy(spo.begin(), spo.end(), ostream_iterator<int>(result2, " "));
      readstring = result2.str();
      readstring = readstring.substr(0, readstring.length()-1);
      pReadCharacteristic->setValue(readstring);
      spo.clear();
      HeaderState = 6;
    }
  }
  else if(HeaderState == 6){
    connectCheck();
    if(writeValue[0] == 'E'){
      copy(temp.begin(), temp.end(), ostream_iterator<int>(result3, " "));
      readstring = result3.str();
      readstring = readstring.substr(0, readstring.length()-1);
      pReadCharacteristic->setValue(readstring);
      temp.clear();
      HeaderState = 7;
    }
  }
  else if(HeaderState == 7){
    }
}

// TODO: Run for 60s then run again for 10 to get temp. Need better flags for control.