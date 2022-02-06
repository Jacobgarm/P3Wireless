/********
  Gruppe 5
  Server med sensorer
  Benytter BLE til at notify clienten om ny data
*********/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <DHT.h>

//Sensorer
#define DHTPIN 4
#define DHTTYPE DHT22   // DHT 22  (AM2302)

DHT dht(DHTPIN, DHTTYPE);

#define MQPIN 14

float temp=20;
float hum=60;
float aq=400;

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 3000;

//Bluetooth
bool deviceConnected = false;

//BLE server name
#define bleServerName "GRP5_ESP32_SERVER"

//UUID's (version 1)
#define SERVICE_UUID "26cac62c-71f3-11ec-90d6-0242ac120003"
#define TEMPERATURE_UUID "3ba9eadc-71f3-11ec-90d6-0242ac120003"
#define HUMIDITY_UUID "453ec176-71f3-11ec-90d6-0242ac120003"
#define AIRQUALITY_UUID "6b033c20-71f3-11ec-90d6-0242ac120003"

// Temperature Characteristic and Descriptor
BLECharacteristic bmeTemperatureCelsiusCharacteristics(TEMPERATURE_UUID, BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmeTemperatureCelsiusDescriptor(BLEUUID((uint16_t)0x2902));


// Humidity Characteristic and Descriptor
BLECharacteristic bmeHumidityCharacteristics(HUMIDITY_UUID, BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmeHumidityDescriptor(BLEUUID((uint16_t)0x2903));

// Air Quality Characteristic and Descriptor
BLECharacteristic bmeAirqualityCharacteristics(AIRQUALITY_UUID, BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmeAirqualityDescriptor(BLEUUID((uint16_t)0x2901));

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

  //Start the DHT22
  dht.begin();

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
      temp = dht.readTemperature();

      // Read humidity
      hum = dht.readHumidity();

      // Read air quality
      aq = analogRead(MQPIN);
  
      //Notify temperature reading from DHT sensor
      static char temperatureCTemp[6];
      dtostrf(temp, 6, 2, temperatureCTemp);
      //Set temperature Characteristic value and notify connected client
      bmeTemperatureCelsiusCharacteristics.setValue(temperatureCTemp);
      bmeTemperatureCelsiusCharacteristics.notify();
      Serial.print("Temperature Celsius: ");
      Serial.print(temp);
      Serial.print(" ºC");

      
      //Notify humidity reading from DHT
      static char humidityTemp[6];
      dtostrf(hum, 6, 2, humidityTemp);
      //Set humidity Characteristic value and notify connected client
      bmeHumidityCharacteristics.setValue(humidityTemp);
      bmeHumidityCharacteristics.notify();   
      Serial.print(" - Humidity: ");
      Serial.print(hum);
      Serial.print(" %");

      //Notify airquality reading from MQ
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
