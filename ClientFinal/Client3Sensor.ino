/********
  Gruppe 5
  Client med display, lyd og SD
  Benytter BLE til at modtager data fra serveren
*********/

#include <BLEDevice.h>
#include <LiquidCrystal.h>
#include <SD.h>
#include <FS.h>
#include <SPI.h>
#include <XT_DAC_Audio.h>

unsigned long lastUpdate = 0;

//Variables to store temperature and humidity
char* temperatureChar;
char* humidityChar;
char* airqualityChar;

String temperatureStr = "111";
String humidityStr = "222";
String airqualityStr = "333";

//Buttons
int buttonPins[] = {16, 4, 17, 2,15};
unsigned long buttonInterval = 1000;
unsigned long lastPressed[] = {0, 0, 0, 0, 0};
int lastValues[] = {0,0,0,0,0};

//Loggging
bool loggingEnabled = true;
unsigned long loggingInterval = 10000;
unsigned long lastLogged = 0;
const char* loggingPath = "/data.csv";

//Alarm Thresholds
float maxT = 28;
float minT = 15;
float maxH = 80;
float minH = 15;
float maxAQ = 1600;
float minAQ = 100;

//Audio
bool alarmOn = true;
int8_t PROGMEM AlarmNotes[] = {
  -50,-50,-57,-57,-59,-59,-57,8,
  -55,-55,-54,-54,-52,-52,-50,8,
  -127
};
int8_t PROGMEM PingNotes[] = {
  -70,-70,8,-127
};

XT_DAC_Audio_Class DacAudio(25,0);
XT_MusicScore_Class AlarmSound(AlarmNotes,156,1);
XT_MusicScore_Class PingSound(PingNotes,156,1);

//LCD Display
const int rs = 13, en = 12, d4 = 14, d5 = 27, d6 = 33, d7 = 32;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
String screenState = "booting";

void soundAlarm() {
  DacAudio.Play(&AlarmSound);
}

void soundPing() {
  DacAudio.Play(&PingSound);
}

void switchAlarm() {
  alarmOn = !alarmOn;
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Alarm is now");
  lcd.setCursor(0,2);
  if (alarmOn) {
    lcd.print("unmuted!");
    soundPing();
  }
  else
    lcd.print("muted!");
  screenState = "alarm";
  lastUpdate = millis();
}

void switchLogging() {
  loggingEnabled = !loggingEnabled;
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Logging is now");
  lcd.setCursor(0,2);
  if (loggingEnabled)
    lcd.print("enabled!");
  else
    lcd.print("disabled!");
  screenState = "log";
  lastUpdate = millis();
}

void displaySensorData(String datatype) {
  lcd.clear();
  lcd.setCursor(0,1);
  if (datatype == "temp") {
    lcd.print("The temperature is:");
    lcd.setCursor(0,2);
    lcd.print(" " + temperatureStr + " C"); 
  }
  else if (datatype == "hum") {
    lcd.print("The humidity is:");
    lcd.setCursor(0,2);
    lcd.print(" " + humidityStr + "%"); 
  }
  else if (datatype == "aq") {
    lcd.print("The airqualtiy is:");
    lcd.setCursor(0,2);
    lcd.print(" " + airqualityStr + " units"); 
  }
  screenState = "singledata";
  lastUpdate = millis();
}

void displayDefault() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Sensor data base");
  lcd.setCursor(0,1);
  lcd.print("Press to see data");
  lcd.setCursor(0,2);
  if (loggingEnabled)
    lcd.print("Logging is enabled");
  else
    lcd.print("Logging is disabled");
  lcd.setCursor(0,3);
  if (alarmOn)
    lcd.print("Alarm is not muted");
  else
    lcd.print("Alarm is muted");
  screenState = "default";
  lastUpdate = millis();
}

void logData(){
  Serial.printf("Logging to file: %s\n", loggingPath);

  File file = SD.open(loggingPath, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for logging");
    return;
  }
  String message = String(millis()) + "," + temperatureStr + "," + humidityStr + "," + airqualityStr + "\n";
  if(file.print(message)){
      Serial.println("Data logged");
  } else {
    Serial.println("Logging failed");
  }
  file.close();
}

void playAudio(char* name) {
    Serial.printf("Playing audio: %s\n", name);
    File file = SD.open("Lyd/" + String(name) + ".wav");
    Serial.println("File opened");
    int len = file.size();
    byte bytes[len] = "";
    file.read(bytes, len);
    //for (int i = 0; i < len; i++) {
    //   bytis[i] = file.read();
    //}
    Serial.println("Array filled");
    XT_Wav_Class sound(bytes);
    Serial.println("Class made");   
    DacAudio.Play(&sound);
    file.close();
    
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
  Serial.print(" ->");
  Serial.print(temperatureStr); 
  Serial.print(" C");


  //print humdidity to serial
  Serial.print(" Humidity:");
  Serial.print(humidityChar); 
  Serial.print(" ->"); 
  Serial.print("%");

  //print airquality to serial
  Serial.print(" Air Quality:");
  Serial.print(airqualityChar); 
  Serial.print(" ->"); 
  Serial.print(airqualityStr); 
  Serial.println(" units");
}

void setup() {
  //Start serial communication
  Serial.begin(9600);
  for (int i = 0; i < 5; i++)
    pinMode(buttonPins[i], INPUT);

  // set up the LCD's number of columns and rows:
  lcd.begin(20, 4);
  lcd.noCursor();
  lcd.noBlink();
  lcd.setCursor(0,1);
  lcd.print("Booting up...");

  //Initialze the SD-card. If it fails, disable logging
  if(!SD.begin(5)){
    Serial.println("Card Mount Failed. Logging disabled");
    loggingEnabled = false;
  }

  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached. Logging disabled");
   loggingEnabled = false;
  }
  
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
      displayDefault();
    } else {
      Serial.println("We have failed to connect to the server; Restart your device to scan for nearby BLE server again.");
    }
    doConnect = false;
  }

  if (!connected)
    return;

  // Fill the audiobuffer every loop, so sound continues to play
  DacAudio.FillBuffer();
  
  //if new temperature readings are available, write to Serial
  if (newTemperature && newHumidity && newAirquality){
    newTemperature = false;
    newHumidity = false;
    newAirquality = false;
    temperatureStr = String(temperatureChar).substring(0,6);
    humidityStr = String(humidityChar).substring(0,6);
    airqualityStr = String(airqualityChar).substring(0,6);
    printReadings();

    //if values exceed thresholds, activate alarm
    if (alarmOn) {
      if (atof(temperatureChar) < minT || atof(temperatureChar) > maxT) {
        soundAlarm();
        displaySensorData("temp");
      }
    
      if (atof(humidityChar) < minH || atof(humidityChar) > maxH) {
        soundAlarm();
        displaySensorData("hum");
      }
    
      if (atof(airqualityChar) < minAQ || atof(airqualityChar) > maxAQ) {
        soundAlarm();
        displaySensorData("aq");
      }
    }
  }
  
  //if logging is enabled and waiting is over, log data
  if (loggingEnabled && millis() - lastLogged > loggingInterval) {
    //Log the data
    logData();
    lastLogged = millis();
  }

  //if no buttons have been pressed for 5 seconds, return to default
  if (screenState != "default" && millis() - lastUpdate > 4000) {
    displayDefault();
  }


  for (int i = 0; i < 5; i++) {
    int val = digitalRead(buttonPins[i]);
    Serial.print(val);
    Serial.print(" ");
    if (millis() - lastPressed[i] > buttonInterval && val == true && lastValues[i] == 1) {
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
          switchAlarm();
      }
      lastValues[i] = val;
  }
  Serial.println();
}
