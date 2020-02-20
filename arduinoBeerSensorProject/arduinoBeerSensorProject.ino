/*--------------INLCUDES--------------*/
//Monitor
#include <LiquidCrystal_I2C.h>
//Memory
#include <SPI.h>
#include <SD.h> //include the SD library
//Sensor
#include <DHT.h>
//RTC
#include <Rtc_Pcf8563.h>

/*--------------DEFINES--------------*/
//Relè
#define RELE1PIN 2
#define RELE2PIN 3

//Memory
#define SS_PIN 10

//Sensor
#define DHT1PIN 6     // what pin we're connected to
#define DHT2PIN 7     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)

/*--------------VARIABLES--------------*/
//Relè
bool Rele1On = false;
bool Rele2On = false;

//Monitor
LiquidCrystal_I2C lcd(0x27, 16, 2);
bool monitorDate = true;

//Memory
byte inByte;
bool sdInitSuccess = false; //card init status
File myFile;
int lineCounter = 0;
bool stopRequired = false;
int memoryWritingPeriod = 10;
int minuteWriting = -1;

//Sensor
DHT dht1(DHT1PIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino
DHT dht2(DHT2PIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino
float hum;  //Stores humidity value
float temp; //Stores temperature value
float DEFAULT_RELE1_LOW_TEMP  = 19.00;
float DEFAULT_RELE1_HIGH_TEMP = 21.00;
float DEFAULT_RELE2_LOW_TEMP  = 19.00;
float DEFAULT_RELE2_HIGH_TEMP = 21.00;

//RTC
Rtc_Pcf8563 rtc;
String rtcTime;
String rtcDate;

/*--------------SETUP--------------*/
void setup() {
  //Debug
  Serial.begin(9600);
  while (!Serial) {
    ; //wait for the serial port to connect.
  }

  //RTC
  //rtc.initClock();
  //rtc.setDate(16,1,2,0,20); 
  //rtc.setTime(22,24,1);
        
  //Monitor
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  //Memory
  pinMode(SS_PIN, OUTPUT);
  delay(1000);
  sdInitSuccess = initSD();

  //Relè
  pinMode(RELE1PIN, OUTPUT);
  pinMode(RELE2PIN, OUTPUT);
  digitalWrite(RELE1PIN, HIGH);
  digitalWrite(RELE2PIN, HIGH);
  Serial.println("Relè 1 OFF");
  Serial.println("Relè 2 OFF");
  Rele1On = false;
  Rele2On = false;

  //Sensor
  dht1.begin();
  dht2.begin();

}

/*--------------LOOP--------------*/
void loop() {
  int minutesNow = 0;

  //Getting Time & Date
  rtcTime = rtc.formatTime(RTCC_TIME_HM);
  rtcDate = rtc.formatDate(RTCC_DATE_WORLD);

  if(monitorDate){
    lcd.clear();
    typewriting(rtcDate + " " + rtcTime);
    delay(2000);
    lcd.clear();
    lcd.setCursor(7,0);
    typewriting("--");
    lcd.setCursor(7,1);
    typewriting("--");
    monitorDate = false;
  }
  
  Serial.println("-------");
  printDebugTime();
  
  //Getting sensor 1 value
  hum = dht1.readHumidity();
  temp= dht1.readTemperature();
  //Debug write
  Serial.print("DHT22 1: ");
  printDebugTempHum();
  //Manage Rele
  if (manageRele(1, temp, DEFAULT_RELE1_LOW_TEMP, DEFAULT_RELE1_HIGH_TEMP)){
    saveDataOnFile();
  }

  //Monitor write
  lcd.setCursor(0,0);
  typewriting("T1:" + getStringFromFloat(temp));
  lcd.setCursor(0,1);
  typewriting("H1:" + getStringFromFloat(hum));
 
  
  //Getting sensor 2 value
  hum = dht2.readHumidity();
  temp= dht2.readTemperature();
  //Debug write
  Serial.print("DHT22 2: ");
  printDebugTempHum();
  //Manage Rele
  if(manageRele(2, temp, DEFAULT_RELE2_LOW_TEMP, DEFAULT_RELE2_HIGH_TEMP)){
    saveDataOnFile();
  }
  
  //Monitor write
  lcd.setCursor(9,0);
  typewriting("T2:" + getStringFromFloat(temp));
  lcd.setCursor(9,1);
  typewriting("H2:" + getStringFromFloat(hum));

  minutesNow = rtc.getMinute();
  if(minuteWriting == -1 || minutesNow == minuteWriting){
    saveDataOnFile();
    minuteWriting = (minutesNow + memoryWritingPeriod) % 60;
  }
  delay(5000);
}

/*--------------FUNCTIONS--------------*/

bool initSD(void){
  Serial.println("Initializing SD Card..");
  if (!SD.begin(10)) { //using pin 10 (SS)
    Serial.println("Initialization failed!");
    Serial.println();
    return false;
  }
  else {
    Serial.println("Intitialization success.");
    Serial.println();
    return true;
  }
}

void saveDataOnFile(void){
  
  if (sdInitSuccess) {
    myFile = SD.open("TEST.txt", FILE_WRITE);
    if (myFile) {
      Serial.println("File opened successfully.");
      Serial.println("Writing to TEST.text");
      myFile.println(rtcTime + "-" + rtcDate + "; DHT1;" + String(dht1.readTemperature()) + ";" + String(dht1.readHumidity()) + ";" + Rele1On);
      myFile.println(rtcTime + "-" + rtcDate + "; DHT2;" + String(dht2.readTemperature()) + ";" + String(dht2.readHumidity()) + ";" + Rele1On);
      myFile.close(); //this writes to the card
      Serial.println("Done");
      Serial.println();
      lineCounter++;
    } else { //else show error
      Serial.println("Error opeing file.\n");
    }
  }
  else if (!sdInitSuccess) { //if not already initialized
      sdInitSuccess = initSD();
  }
}

void typewriting(String messaggio) {
  int lunghezza = messaggio.length();
  for (int i = 0; i < lunghezza; i++) {
    lcd.print(messaggio[i]);
  }
}

String getStringFromFloat(float var){
  return String(var).substring(0, String(var).length() - 1);
}

void printDebugTempHum(void) {
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(" %, Temp: ");
  Serial.print(temp);
  Serial.println(" Celsius");
}

void printDebugTime(void){
  Serial.print(rtcDate);
  Serial.print(" ");
  Serial.print(rtcTime);
  Serial.print("\r\n");
}

bool manageRele(int rele, float temp, float lowTemp, float highTemp){
  int retVal = true;
  
  if(isnan(temp)){
    Serial.println("TEMP reading error sensor " + String(rele));
    retVal = false;
    return retVal;
  }
  
  switch(rele){
    case 1:
        if(temp < lowTemp && Rele1On == false){
          digitalWrite(RELE1PIN, LOW);
          Serial.println("Relè 1 ON");
          Rele1On = true;
        }
        else if(temp > highTemp && Rele1On == true)
        {
          digitalWrite(RELE1PIN, HIGH);
          Serial.println("Relè 1 OFF");
          Rele1On = false;
        }
        else
        {
          retVal = false;
        }
      break;
    case 2:
        if(temp < lowTemp && Rele2On == false){
          digitalWrite(RELE2PIN, LOW);
          Serial.println("Relè 2 ON");
          Rele2On = true;
        }
        else if(temp > highTemp && Rele2On == true)
        {
          digitalWrite(RELE2PIN, HIGH);
          Serial.println("Relè 2 OFF");
          Rele2On = false;
        }
        else
        {
          retVal = false;
        }
      break;
    default:
      Serial.println("ERROR - Rele not available");
      retVal = false;
      break;
  }

  return retVal;
}
