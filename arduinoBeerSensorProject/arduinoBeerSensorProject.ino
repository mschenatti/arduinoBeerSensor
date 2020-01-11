/*--------------INLCUDES--------------*/
//Monitor
#include <LiquidCrystal_I2C.h>
//Memory
#include <SPI.h>
#include <SD.h> //include the SD library
//Sensor
#include <DHT.h>;
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
bool stopRequired = false;
bool selectRequired = false;
bool changeRequired = false;
typedef enum variableOnChange{
  DAY,
  MONTH,
  YEAR,
  HOURS,
  MINUTES
}eVariableOnChange;
eVariableOnChange variableOnChange = DAY;
typedef struct dateVar{
  int day;
  int month;
  int year;
  int hours;
  int minutes;
}sDateVar;
sDateVar date;
//Monitor
LiquidCrystal_I2C lcd(0x27, 16, 2);
String riga1 = "gg/mm/aaaa hh:mm";
String riga2 = "T:--C H:--%";

//Memory
byte inByte;
bool sdInitSuccess = false; //card init status
File myFile;
int lineCounter = 0;

//Sensor
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino
int chk;
float hum;  //Stores humidity value
float temp; //Stores temperature value

//RTC
Rtc_Pcf8563 rtc;
String rtcTime;
String rtcDate;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; //wait for the serial port to connect.
  }

  lcd.init();
  lcd.backlight();

  pinMode(SS_PIN, OUTPUT);
  pinMode(LED_FS_W_PIN, OUTPUT);
  digitalWrite(LED_FS_W_PIN, LOW);

  pinMode(LED_STOP_PIN, OUTPUT);
  digitalWrite(LED_STOP_PIN, LOW);

  pinMode(INPUT_STOP_PIN, INPUT);

  dht.begin();

  /*rtc.initClock();
  //set a time to start with.
  //day, weekday, month, century(1=1900, 0=2000), year(0-99)
  rtc.setDate(10, 1, 1, 0, 20); 
  //hr, min, sec
  rtc.setTime(21, 41, 0);*/
}

void loop() {
  stopRequired = digitalRead(INPUT_STOP_PIN);
  if(stopRequired){
    Serial.println("STOP REQUIRED");
    digitalWrite(LED_STOP_PIN, HIGH);
    while(1){
      selectRequired = digitalRead(INPUT_SELECT);
      changeRequired = digitalRead(INPUT_CHANGE);
      Serial.println("selectRequired = " + String(selectRequired));
      Serial.println("changeRequired = " + String(changeRequired));
      if(selectRequired && changeRequired){
        Serial.println("DATE SETUP REQUIRED");
        digitalWrite(LED_FS_W_PIN, HIGH);
        delay(200);
        while(1){
          lcdPrintVariable(variableOnChange, date.day);
          switch(variableOnChange){
            case DAY:
              if(changeRequired){
                date.day++;
              }
              if(changeRequired){
                variableOnChange = MONTH;
              }
          }
          delay(500);
        }
      }
    }
  }else{
    digitalWrite(LED_STOP_PIN, LOW);
  }
  
  //Getting sensor value
  hum = dht.readHumidity();
  temp= dht.readTemperature();
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(" %, Temp: ");
  Serial.print(temp);
  Serial.println(" Celsius");

  //Getting Time & Date
  rtcTime = rtc.formatTime();
  rtcDate = rtc.formatDate();

  riga1 = rtcDate + " " + rtcTime;
  //Monitor write
  lcd.clear();
  lcd.setCursor(0,0);
  typewriting(riga1);
  lcd.setCursor(0,1);
  riga2 = "T:" + getStringFromFloat(temp) + "C H:" + getStringFromFloat(hum) + "%";
  typewriting(riga2);

  Serial.print(rtc.formatTime());
  Serial.print("\r\n");
  Serial.print(rtc.formatDate());
  Serial.print("\r\n");

  if(!stopRequired){
    saveDataOnFile();
  }
  
  delay(5000);
}

void saveDataOnFile(void){
  digitalWrite(LED_FS_W_PIN, HIGH);
  if (sdInitSuccess) {
    Serial.println("Already initialized.");
    Serial.println();
    myFile = SD.open("TEST.txt", FILE_WRITE);
    if (myFile) {
      Serial.println("File opened successfully.");
      Serial.println("Writing to TEST.text");
      myFile.print("Line "); myFile.println(lineCounter);
      myFile.close(); //this writes to the card
      Serial.println("Done");
      Serial.println();
      lineCounter++;
    } else { //else show error
      Serial.println("Error opeing file.\n");
    }
  }
  else if (!sdInitSuccess) { //if not already initialized
    Serial.println("Initializing SD Card..");
    if (!SD.begin(10)) { //using pin 10 (SS)
      Serial.println("Initialization failed!");
      Serial.println();
      sdInitSuccess = false;
      return;
    }
    else {
      Serial.println("Intitialization success.");
      Serial.println();
      sdInitSuccess = true;
    }
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

String getVaribaleOnChangeString(eVariableOnChange variable){
  switch(variable){
    case DAY: return "DAY";break;
    case MONTH: return "DAY";break;
    case YEAR: return "DAY";break;
    case HOURS: return "DAY";break;
    case MINUTES: return "DAY";break;
    default: return "ERROR"; break;
  }
}

void lcdPrintVariable(eVariableOnChange variable, int num){
  String sVariable = getVaribaleOnChangeString(variable);
  lcd.clear();
  lcd.setCursor(0,0);
  typewriting(sVariable + " " + String(num));
}

