/********
  Gruppe 5
  Client med display
*********/

#include <BLEDevice.h>
#include <LiquidCrystal.h>
//#include <SD.h>

//Variables to store temperature and humidity
char* temperatureChar;
char* humidityChar;
char* airqualityChar;

//Buttons
int buttonPins[] = {12, 13, 14, 15,7};
unsigned long buttonInterval = 500;
unsigned long lastPressed[] = {0, 0, 0, 0, 0};

//Loggging
bool loggingEnabled = true;
unsigned long loggingInterval = 10000;
unsigned long lastLogged = 0;

//Audio
bool isMuted = false;

//LCD Display
const int rs = 0, en = 16, d4 = 17, d5 = 5, d6 = 18, d7 = 19;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

static void switchMute() {
  isMuted = !isMuted;
  lcd.clear();
  lcd.setCursor(0,1);
  if (isMuted)
    lcd.print("Audio is now muted!");
  else
    lcd.print("Audio is no longer muted!");
}

static void switchLogging() {
  loggingEnabled = !loggingEnabled;
  lcd.clear();
  lcd.setCursor(0,1);
  if (isMuted)
    lcd.print("Logging is now enabled!");
  else
    lcd.print("Logging is now disabled!");
}

static void displaySensorData(String datatype) {
  lcd.clear();
  lcd.setCursor(0,1);
  if (datatype == "temp") {
    lcd.print("The temperature is:");
    lcd.setCursor(0,2);
    lcd.print(" " + String(temperatureChar) + " C"); 
  }
  else if (datatype == "hum") {
    lcd.print("The humidity is:");
    lcd.setCursor(0,2);
    lcd.print(" " + String(humidityChar) + "%"); 
  }
  else if (datatype == "aq") {
    lcd.print("The airqualtiy is:");
    lcd.setCursor(0,2);
    lcd.print(" " + String(airqualityChar) + " units"); 
  }
  
}

//Bluetooth
//BLE Server name (the other ESP32 name running the server sketch)
#define bleServerName "GRP5_ESP32_SERVER"

//UUID's (version 1)
#define SERVICE_UUID "26cac62c-71f3-11ec-90d6-0242ac120003"
#define TEMPERATURE_UUID "3ba9eadc-71f3-11ec-90d6-0242ac120003"
#define HUMIDITY_UUID "453ec176-71f3-11ec-90d6-0242ac120003"
#define AIRQUALITY_UUID "6b033c20-71f3-11ec-90d6-0242ac120003"


/* UUID's of the service, characteristic that we want to read*/
// BLE Service
static BLEUUID bmeServiceUUID(SERVICE_UUID);

// BLE Characteristics
//Temperature Celsius Characteristic
static BLEUUID temperatureCharacteristicUUID(TEMPERATURE_UUID);

// Humidity Characteristic
static BLEUUID humidityCharacteristicUUID(HUMIDITY_UUID);

// Air Quality Characteristic
static BLEUUID airqualityCharacteristicUUID(AIRQUALITY_UUID);

//Flags stating if should begin connecting and if the connection is up
static boolean doConnect = false;
static boolean connected = false;

//Address of the peripheral device. Address will be found during scanning...
static BLEAddress *pServerAddress;
 
//Characteristicd that we want to read
static BLERemoteCharacteristic* temperatureCharacteristic;
static BLERemoteCharacteristic* humidityCharacteristic;
static BLERemoteCharacteristic* airqualityCharacteristic;

//Activate notify
const uint8_t notificationOn[] = {0x1, 0x0};
const uint8_t notificationOff[] = {0x0, 0x0};



//Flags to check whether new temperature and humidity readings are available
boolean newTemperature = false;
boolean newHumidity = false;
boolean newAirquality = false;

//Connect to the BLE Server that has the name, Service, and Characteristics
bool connectToServer(BLEAddress pAddress) {
   BLEClient* pClient = BLEDevice::createClient();
 
  // Connect to the remove BLE Server.
  pClient->connect(pAddress);
  Serial.println(" - Connected to server");
 
  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(bmeServiceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(bmeServiceUUID.toString().c_str());
    return (false);
  }
 
  // Obtain a reference to the characteristics in the service of the remote BLE server.
  temperatureCharacteristic = pRemoteService->getCharacteristic(temperatureCharacteristicUUID);
  humidityCharacteristic = pRemoteService->getCharacteristic(humidityCharacteristicUUID);
  airqualityCharacteristic = pRemoteService->getCharacteristic(airqualityCharacteristicUUID);

  if (temperatureCharacteristic == nullptr || humidityCharacteristic == nullptr || airqualityCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID");
    return false;
  }
  Serial.println(" - Found our characteristics");
 
  //Assign callback functions for the Characteristics
  temperatureCharacteristic->registerForNotify(temperatureNotifyCallback);
  humidityCharacteristic->registerForNotify(humidityNotifyCallback);
  airqualityCharacteristic->registerForNotify(airqualityNotifyCallback);
  return true;
}

//Callback function that gets called, when another device's advertisement has been received
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName() == bleServerName) { //Check if the name of the advertiser matches
      advertisedDevice.getScan()->stop(); //Scan can be stopped, we found what we are looking for
      pServerAddress = new BLEAddress(advertisedDevice.getAddress()); //Address of advertiser is the one we need
      doConnect = true; //Set indicator, stating that we are ready to connect
      Serial.println("Device found. Connecting!");
    }
  }
};
 
//When the BLE Server sends a new temperature reading with the notify property
static void temperatureNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
    uint8_t* pData, size_t length, bool isNotify) {
  //store temperature value
  temperatureChar = (char*)pData;
  newTemperature = true;
}

//When the BLE Server sends a new humidity reading with the notify property
static void humidityNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
     uint8_t* pData, size_t length, bool isNotify) {
  //store humidity value
  humidityChar = (char*)pData;
  newHumidity = true;
}

//When the BLE Server sends a new aiqual reading with the notify property
static void airqualityNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
     uint8_t* pData, size_t length, bool isNotify) {
  //store humidity value
  airqualityChar = (char*)pData;
  newAirquality = true;
}

//function that prints the latest sensor readings to serial
void printReadings(){

  //print temperature to serial
  Serial.print("Temperature:");
  Serial.print(temperatureChar);
  Serial.print("C");


  //print humdidity to serial
  Serial.print(" Humidity:");
  Serial.print(humidityChar); 
  Serial.print("%");

  //print airquality to serial
  Serial.print(" Air Quality:");
  Serial.print(airqualityChar); 
  Serial.println(" ");
}

void setup() {
  //Start serial communication
  Serial.begin(9600);
  for (int i = 0; i < 5; i++)
    pinMode(buttonPins[i], INPUT);

  // set up the LCD's number of columns and rows:
  lcd.begin(20, 4);
  
  Serial.println("Starting Arduino BLE Client application...");

  //Init BLE device
  BLEDevice::init("");
 
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 30 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30);
}

void loop() {
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer(*pServerAddress)) {
      Serial.println("We are now connected to the BLE Server.");
      //Activate the Notify property of each Characteristic
      temperatureCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
      humidityCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
      airqualityCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
      connected = true;
    } else {
      Serial.println("We have failed to connect to the server; Restart your device to scan for nearby BLE server again.");
    }
    doConnect = false;
  }
  //if new temperature readings are available, print in the OLED
  if (newTemperature && newHumidity && newAirquality){
    newTemperature = false;
    newHumidity = false;
    newAirquality = false;
    printReadings();
  }

  if (loggingEnabled && millis() - lastLogged > loggingInterval) {
      //Log the data
      
      lastLogged = millis();
  }

  for (int i = 0; i < 5; i++) {
    if (millis() - lastPressed[i] > buttonInterval && analogRead(buttonPins[i]) > 512) {
        lastPressed[i] = millis();
        if (i == 0)
          displaySensorData("temp");
        else if (i == 1)
          displaySensorData("hum");
        else if (i == 2)
          displaySensorData("aq");
        else if (i == 3)
          switchLogging();
        else if (i == 4)
          switchMute();
      }
  }

  
  delay(10); // Delay a second between loops.
}
