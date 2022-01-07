#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLE2902.h>
#include <BLEServer.h>

#define serviceUUID "11265ab0-f3a7-451b-b2ab-3a9025f12250"
#define characteristicUUID "d282db32-f956-4e72-8758-1dd1ecbb8d49"

#define serviceID BLEUUID((uint16_t)0x1700)

uint8_t value = 0;

BLECharacteristic customCharacteristic(
  BLEUUID((uint16_t)0x1700), 
  BLECharacteristic::PROPERTY_READ | 
  BLECharacteristic::PROPERTY_NOTIFY
);

void setup() {


Serial.begin(115200);
  
// Initializer BLE environment
BLEDevice::init("Den seje");
Serial.println("Name of device is:'Den seje'");
// Create server
BLEServer* pServer = BLEDevice::createServer();


// Create service
BLEService* pService = pServer->createService(serviceUUID/*, BLECharacteristic::PROPERTY_READ*/);


// Create Characteristics
  pService->createCharacteristic(&customCharacteristic);

// Set characteristic value
customCharacteristic.addDescriptor(new BLE2902());

// Start Service
pService->start();

// Advertizing
BLEAdvertising* pAdvertising = pServer->getAdvertising();
pAdvertising->addServiceUUID(serviceUUID);
pAdvertising->setScanResponse(true);
pAdvertising->setMinPreferred(0x12);
pAdvertising->start();

}
//Serial.println("Device is now available to the IoT");
void loop() {
  // put your main code here, to run repeatedly:

}
