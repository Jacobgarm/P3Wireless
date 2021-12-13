

// include the library code:
#include <LiquidCrystal.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT22   // DHT 22  (AM2302)

DHT dht(DHTPIN, DHTTYPE);
int R_0 = 945;

#define MQPIN 14

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 0, en = 16, d4 = 17, d5 = 5, d6 = 18, d7 = 19;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

float hum;
float temp;
float AQ;

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(20, 4);
  //turn on the cursor:
  lcd.cursor();
  Serial.begin(9600);
  dht.begin();
}

void loop() {
  hum = dht.readHumidity();
  temp = dht.readTemperature();
  AQ = analogRead(MQPIN);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Humidity: " + String(hum) + " %");
  lcd.setCursor(0,1);
  lcd.print("Temp: " + String(temp) + " C");
  lcd.setCursor(0,2);
  lcd.print("Air quality: " + String(AQ));
  Serial.println(hum);
  Serial.println(temp);
  Serial.println(AQ);
  delay(1000);
}
