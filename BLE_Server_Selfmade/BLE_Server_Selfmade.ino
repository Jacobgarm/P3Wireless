#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define serviceUUID "11265ab0-f3a7-451b-b2ab-3a9025f12250"
#define characteristicUUID "d282db32-f956-4e72-8758-1dd1ecbb8d49"



void setup() {


Serial.begin(115200);
  
// Initializer BLE environment
BLEDevice::init("Den seje gruppe");
Serial.println("Name of device is:'Den seje gruppe'");
// Create server
BLEServer* pServer = BLEDevice::createServer();
// Create service
BLEService* pService = pServer->createService(serviceUUID/*, BLECharacteristic::PROPERTY_READ*/);
// Create Characteristics
BLECharacteristic* pCharacteristic =
  pService->createCharacteristic(characteristicUUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

// Set characteristic value
pCharacteristic->setValue("Tsiamnd");

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
