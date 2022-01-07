#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

uint8_t value = 0;  //the set value function only accepts unsigned 8 bit integers

/* Define the UUID for our Custom Service */
#define serviceID BLEUUID((uint16_t)0x1700)

/* Define our custom characteristic along with it's properties */
BLECharacteristic customCharacteristic(
  BLEUUID("d282db32-f956-4e72-8758-1dd1ecbb8d49"), 
  BLECharacteristic::PROPERTY_READ | 
  BLECharacteristic::PROPERTY_NOTIFY
);

/* This function handles the server callbacks */
bool deviceConnected = false;
class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* MyServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* MyServer) {
      deviceConnected = false;
    }
};

void setup() {
  //Serial.begin(115200);

  // Create and name the BLE Device
  BLEDevice::init("Steen er ikke");

  /* Create the BLE Server */
  BLEServer *MyServer = BLEDevice::createServer();
  MyServer->setCallbacks(new ServerCallbacks());  // Set the function that handles Server Callbacks

  /* Add a service to our server */
  BLEService *customService = MyServer->createService(BLEUUID((uint16_t)0x1700)); //  A random ID has been selected

  /* Add a characteristic to the service */
  customService->addCharacteristic(&customCharacteristic);  //customCharacteristic was defined above

  /* Add Descriptors to the Characteristic*/
  customCharacteristic.addDescriptor(new BLE2902());  //Add this line only if the characteristic has the Notify property

  /* Configure Advertising with the Services to be advertised */
  MyServer->getAdvertising()->addServiceUUID(serviceID);

  // Start the service
  customService->start();

  // Start the Server/Advertising
  MyServer->getAdvertising()->start();

  //Serial.println("Waiting for a Client to connect...");
}

void loop() {

  if (deviceConnected) {
    /* Set the value */
    //TEST ok
    value++;
    customCharacteristic.setValue(&value,1);  // This is a value of a single byte
    customCharacteristic.notify();  // Notify the client of a change
  }
  delay(1000);
}
