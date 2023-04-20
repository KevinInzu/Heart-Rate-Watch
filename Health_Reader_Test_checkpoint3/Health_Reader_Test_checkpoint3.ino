/*
  Health_Reader_Test_checkpoint2.ino
  03/19/23
  Undergraduate final project

  Main running code for the esp32 heartrate watch.

  Some sections of BLE code are based on library example by Neil Kolban
  created by Steve Graves
*/

#define SERVICE_UUID_HR             "7da88714-c6c9-11ed-afa1-0242ac120002" //714
// TODO: might need a 2nd service here for battery

#define CHARACTERISTIC_UUID_WRITE   "7da889bc-c6c9-11ed-afa1-0242ac120002" //9BC
#define CHARACTERISTIC_UUID_READ  "7da88be2-c6c9-11ed-afa1-0242ac120002" //BE2

// BLE libraries
#include <BLEDevice.h>
#include <BLEUtils.h>

// BLE variables
BLEServer* pServer = NULL;
BLECharacteristic* pReadCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// BLE DEBUG STUFF
std::string readData[10] = {"01", "02", "03", "04", "05","06", "07", "08", "09", "0A"}; // THIS WORKS!!! MUST BE A SIMPLE STRING!!

// This class changes the deviceConnected flag when connected or disconnected from the ESP32
class MyServerCallbacks:
  public BLEServerCallbacks {
    void onConnect(BLEServer* pServer){
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer){
      deviceConnected = false;
    }
  };


void setup() {
  ///--- General Esp32 Setup ---///
  Serial.begin(115200);

  ///--- BLE Setup ---///
  
  // Create BLE device
  BLEDevice::init("ESP32_Proto2");

  // Create BLE server on device
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE service
  BLEService *pService = pServer->createService(SERVICE_UUID_HR);

  // Create BLE characteristic(s)
  pReadCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_READ, BLECharacteristic::PROPERTY_READ);
  
  // Start service
  pService->start();

  pServer->getAdvertising()->start();
  Serial.println("Waiting for a client to connect...");

  ///--- MAX30102 Setup ---///

}

// This loop works to constantly cycle the array and allow you to read what's present. Now I need to concatinate a large chunk of data for a SINGLE large read
void loop() {
  for(int i = 0; i < sizeof(readData)/sizeof(readData[0]); i++){
    pReadCharacteristic->setValue(readData[i]);
    delay(500);
  }
  connectCheck();
}



// BLE function for checking if application is connected
void connectCheck(){

  if (!deviceConnected && oldDeviceConnected) {
        delay(1000); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
  }
    // connecting
  if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
  }  
}

//Reference the write example for further reminders of how the read function can be manipulated

//TODO: Need to make a general state handler ex. State for reading data, or being in BLE mode

//TODO: Need to make a debug function that allows me to load any string of values for being sent via BLE

//TODO: Need to make 2 test functions so that I can compare if it's better to save all the data internally on the esp before sending,
//      or if sending it all and having the app deal with it is better.

//TODO: Need to implement a sleep mode for the esp32

//TODO: Need to have 2 directional communication via BLE so that the device can be woken and ran with the mobile application

//TODO: Find out a way to communicate battery percentage to the mobile application

