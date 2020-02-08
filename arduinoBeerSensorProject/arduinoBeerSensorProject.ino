#include <RTClib.h>

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
#define LED_STOP_PIN 4
#define INPUT_STOP_PIN 5
#define INPUT_SELECT 9
#define INPUT_CHANGE 8

//Monitor
//Memory
#define LED_FS_W_PIN 3
#define SS_PIN 10
//Sensor
#define DHTPIN 7     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)

/*--------------VARIABLES--------------*/
//Monitor
LiquidCrystal_I2C lcd(0x27, 16, 2);
String raw1 = "gg/mm/aaaa hh:mm";
String raw2 = "T:--C H:--%";

//Memory
byte inByte;
bool sdInitSuccess = false; //card init status
File myFile;
int lineCounter = 0;
bool stopRequired = false;

//Sensor
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino
int chk;
float hum;  //Stores humidity value
float temp; //Stores temperature value

//RTC
Rtc_Pcf8563 rtc;
String rtcTime;
String rtcDate;
typedef struct dateVar{
  int day;
  int month;
  int year;
  int hours;
  int minutes;
}sDateVar;
sDateVar date;
int minuteWriting = -1;


//TERMISTOR
bool termistorStatus = false;
/*--------------SETUP--------------*/
void setup() {
  //Debug
  Serial.begin(9600);
  while (!Serial) {
    ; //wait for the serial port to connect.
  }

  //Monitor
  lcd.init();
  lcd.backlight();

  //Memory
  pinMode(SS_PIN, OUTPUT);
  pinMode(LED_FS_W_PIN, OUTPUT);
  digitalWrite(LED_FS_W_PIN, LOW);
  delay(1000);
  sdInitSuccess = initSD();

  //Input/Output
  pinMode(LED_STOP_PIN, OUTPUT);
  pinMode(INPUT_STOP_PIN, INPUT);

  //Sensor
  dht.begin();

  //RTC
  if(sdInitSuccess){
    if(SD.exists("DATE.txt")){
      Serial.println("Date setting");
      File dateFile;
      char buf[15];
      String sBuf;
      dateFile = SD.open("DATE.txt", FILE_READ);
      if(dateFile){
        date.day = dateFile.parseInt();
        date.month = dateFile.parseInt();
        date.year = dateFile.parseInt();
        date.hours = dateFile.parseInt();
        date.minutes = dateFile.parseInt();
        dateFile.close();
        rtc.initClock();
        rtc.setDate(date.day, 1, date.month, 0, date.year); 
        rtc.setTime(date.hours, date.minutes, 0);
      }
      Serial.println("Date file removing");
      SD.remove("DATE.txt");
    }
  }

  //TERMISTOR
  digitalWrite(LED_STOP_PIN, LOW);
  Serial.println("Termoresistor OFF");
  termistorStatus = false;
}

/*--------------LOOP--------------*/
void loop() {
//  stopRequired = digitalRead(INPUT_STOP_PIN);
//  if(stopRequired){
//    Serial.println("STOP REQUIRED");
//    while(stopRequired){
//      stopRequired = digitalRead(INPUT_STOP_PIN);
//      delay(5000);
//    }
//    Serial.println("START REQUIRED");
//  }else{
//  }
  
  //Getting sensor value
  hum = dht.readHumidity();
  temp= dht.readTemperature();

  if(temp < 18 && termistorStatus == false){
    digitalWrite(LED_STOP_PIN, HIGH);
    Serial.println("Termoresistor ON");
    termistorStatus = true;
  }
  else if(temp > 19 && termistorStatus == true)
  {
    digitalWrite(LED_STOP_PIN, LOW);
    Serial.println("Termoresistor OFF");
    termistorStatus = false;
  }
  else
  {
    //Do nothing
  }
  //Getting Time & Date
  rtcTime = rtc.formatTime(RTCC_TIME_HM);
  rtcDate = rtc.formatDate(RTCC_DATE_WORLD);
  
  //Monitor write
  lcd.clear();
  lcd.setCursor(0,0);
  raw1 = rtcDate + " " + rtcTime;
  typewriting(raw1);
  lcd.setCursor(0,1);
  raw2 = "T:" + getStringFromFloat(temp) + "C H:" + getStringFromFloat(hum) + "%";
  typewriting(raw2);

  //Debug write
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(" %, Temp: ");
  Serial.print(temp);
  Serial.println(" Celsius");
  Serial.print(rtcTime);
  Serial.print("\r\n");
  Serial.print(rtcDate);
  Serial.print("\r\n");

  date.minutes=rtc.getMinute();
  if(minuteWriting == -1 || date.minutes == minuteWriting){
    saveDataOnFile();
    minuteWriting = (date.minutes + 10) % 60;
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
  digitalWrite(LED_FS_W_PIN, HIGH);
  if (sdInitSuccess) {
    myFile = SD.open("TEST.txt", FILE_WRITE);
    if (myFile) {
      Serial.println("File opened successfully.");
      Serial.println("Writing to TEST.text");
      myFile.print(raw1 + ";" + temp + ";" + hum + ";" + termistorStatus); myFile.println();
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
  digitalWrite(LED_FS_W_PIN, LOW);
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
