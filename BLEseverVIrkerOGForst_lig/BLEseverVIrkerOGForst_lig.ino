

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>

//Default Temperature is in Celsius
//Comment the next line for Temperature in Fahrenheit
#define Vardi

//BLE server name
#define bleServerName "BME280_ESP32"

float Vardital;


// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

bool deviceConnected = false;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID "91bad492-b950-4226-aa2b-4ede9fa42f59"

// Temperature Characteristic and Descriptor

  BLECharacteristic bmeVardiCharacteristics("cba1d466-344c-4be3-ab3f-189f80dd7518", BLECharacteristic::PROPERTY_NOTIFY);
  BLEDescriptor bmeVardiDescriptor(BLEUUID((uint16_t)0x2902));

// Humidity Characteristic and Descriptor
/*BLECharacteristic bmeHumidityCharacteristics("ca73b3ba-39f6-4ab3-91ae-186dc9577d99", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmeHumidityDescriptor(BLEUUID((uint16_t)0x2903));*/

//Setup callbacks onConnect and onDisconnect
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

/*void initBME(){
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
}*/

void setup() {
  // Start serial communication 
  Serial.begin(115200);

  // Init BME Sensor
  /*initBME():   */

  // Create the BLE Device
  BLEDevice::init(bleServerName);

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *bmeService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristics and Create a BLE Descriptor
  // Temperature
  /*#ifdef temperatureCelsius
    bmeService->addCharacteristic(&bmeTemperatureCelsiusCharacteristics);
    bmeTemperatureCelsiusDescriptor.setValue("BME temperature Celsius");
    bmeTemperatureCelsiusCharacteristics.addDescriptor(&bmeTemperatureCelsiusDescriptor);*/

  // Humidity
  bmeService->addCharacteristic(&bmeVardiCharacteristics);
  bmeVardiDescriptor.setValue("BME vardi");
  bmeVardiCharacteristics.addDescriptor(new BLE2902());
  
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
    
   
        Vardital++;
  
      //Notify temperature reading from BME sensor
      /*#ifdef temperatureCelsius
        static char temperatureCTemp[6];
        dtostrf(temp, 6, 2, temperatureCTemp);
        //Set temperature Characteristic value and notify connected client
        bmeTemperatureCelsiusCharacteristics.setValue(temperatureCTemp);
        bmeTemperatureCelsiusCharacteristics.notify();
        Serial.print("Temperature Celsius: ");
        Serial.print(temp);
        Serial.print(" ºC");
      #else
        static char temperatureFTemp[6];
        dtostrf(tempF, 6, 2, temperatureFTemp);
        //Set temperature Characteristic value and notify connected client
        bmeTemperatureFahrenheitCharacteristics.setValue(temperatureFTemp);
        bmeTemperatureFahrenheitCharacteristics.notify();
        Serial.print("Temperature Fahrenheit: ");
        Serial.print(tempF);
        Serial.print(" ºF");
      #endif*/
      
      //Notify humidity reading from BME
      /*static char humidityTemp[6];
      dtostrf(hum, 6, 2, humidityTemp);*/
      //Set humidity Characteristic value and notify connected client
      bmeVardiCharacteristics.setValue(Vardital);
      bmeVardiCharacteristics.notify();   
      Serial.print(" - Humidity: ");
      Serial.print(Vardital);
      Serial.println(" %");

      delay(1000);
  }
}
