/********
  Gruppe 5
  Server med sensorer
*********/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <DHT.h>

float temp=1;
float hum=2;
float aq=3;

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 3000;

bool deviceConnected = false;

//BLE server name
#define bleServerName "GRP5_ESP3_SERVER2"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID "d1c7319c-6fbd-11ec-90d6-0242ac120003"

// Temperature Characteristic and Descriptor
BLECharacteristic bmeTemperatureCelsiusCharacteristics("cba1d466-344c-4be3-ab3f-189f80dd7518", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmeTemperatureCelsiusDescriptor(BLEUUID((uint16_t)0x2902));


// Humidity Characteristic and Descriptor
BLECharacteristic bmeHumidityCharacteristics("ca73b3ba-39f6-4ab3-91ae-186dc9577d99", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmeHumidityDescriptor(BLEUUID((uint16_t)0x2903));

// Air Quality Characteristic and Descriptor
BLECharacteristic bmeAirqualityCharacteristics("5eda95c2-6fc0-11ec-90d6-0242ac120003", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmeAirqualityDescriptor(BLEUUID((uint16_t)0x2903));

//Setup callbacks onConnect and onDisconnect
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

void setup() {
  // Start serial communication 
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init(bleServerName);

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *bmeService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristics and Create a BLE Descriptor
  // Temperature
  bmeService->addCharacteristic(&bmeTemperatureCelsiusCharacteristics);
  bmeTemperatureCelsiusDescriptor.setValue("BME temperature Celsius");
  bmeTemperatureCelsiusCharacteristics.addDescriptor(new BLE2902());
 

  // Humidity
  bmeService->addCharacteristic(&bmeHumidityCharacteristics);
  bmeHumidityDescriptor.setValue("BME humidity");
  bmeHumidityCharacteristics.addDescriptor(new BLE2902());

  // Air Quality
  bmeService->addCharacteristic(&bmeAirqualityCharacteristics);
  bmeAirqualityDescriptor.setValue("BME airquality");
  bmeAirqualityCharacteristics.addDescriptor(new BLE2902());

  // Start the service
  bmeService->start();
  
  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  if (deviceConnected) {
    if ((millis() - lastTime) > timerDelay) {
      // Read temperature as Celsius
      temp++;

      // Read humidity
      hum++;

      // Read air quality
      aq ++;
  
      //Notify temperature reading from BME sensor
      static char temperatureCTemp[6];
      dtostrf(temp, 6, 2, temperatureCTemp);
      //Set temperature Characteristic value and notify connected client
      bmeTemperatureCelsiusCharacteristics.setValue(temperatureCTemp);
      bmeTemperatureCelsiusCharacteristics.notify();
      Serial.print("Temperature Celsius: ");
      Serial.print(temp);
      Serial.print(" ºC");

      
      //Notify humidity reading from BME
      static char humidityTemp[6];
      dtostrf(hum, 6, 2, humidityTemp);
      //Set humidity Characteristic value and notify connected client
      bmeHumidityCharacteristics.setValue(humidityTemp);
      bmeHumidityCharacteristics.notify();   
      Serial.print(" - Humidity: ");
      Serial.print(hum);
      Serial.println(" %");

      //Notify airquality reading from BME
      static char airqualityTemp[6];
      dtostrf(aq, 6, 2, airqualityTemp);
      //Set air quality Characteristic value and notify connected client
      bmeAirqualityCharacteristics.setValue(airqualityTemp);
      bmeAirqualityCharacteristics.notify();   
      Serial.print(" - Air quality: ");
      Serial.println(aq);
      
      lastTime = millis();
    }
  }
}
