#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>


void setup() {
BLEScan* pMyScan = BLEDevice::getScan();
pMyScan.Start(15); // Scans for 15 seconds
}

void loop() {
  // put your main code here, to run repeatedly:

}
