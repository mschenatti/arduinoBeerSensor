/*--------------INLCUDES--------------*/
//Monitor
#include <LiquidCrystal_I2C.h>
//Memory
#include <SPI.h>
//Sensor
#include <DHT.h>

/*--------------DEFINES--------------*/
//Relè
#define RELE1PIN 2
#define RELE2PIN 3

//Sensor
#define DHT1PIN 6     // what pin we're connected to
#define DHT1POWERPIN 4// what pin we're connected to
#define DHT2PIN 7     // what pin we're connected to
#define DHT2POWERPIN 5// what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define MAX_NAN 5

/*--------------VARIABLES--------------*/
//Relè
bool Rele1On = false;
bool Rele2On = false;

//Monitor
LiquidCrystal_I2C lcd(0x27, 16, 2);
bool monitorDate = true;

//Sensor
int nan1_counter = 0;
int nan2_counter = 0;
bool dht1_fail = false;
bool dht2_fail = false;
DHT dht1(DHT1PIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino
DHT dht2(DHT2PIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino
float hum1;  //Stores humidity value
float temp1; //Stores temperature value
float hum2;  //Stores humidity value
float temp2; //Stores temperature value
float DEFAULT_RELE1_LOW_TEMP  = 19.00;
float DEFAULT_RELE1_HIGH_TEMP = 21.00;
float DEFAULT_RELE2_LOW_TEMP  = 19.00;
float DEFAULT_RELE2_HIGH_TEMP = 21.00;

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
  lcd.clear();

  //Relè
  pinMode(RELE1PIN, OUTPUT);
  pinMode(RELE2PIN, OUTPUT);
  digitalWrite(RELE1PIN, HIGH);
  digitalWrite(RELE2PIN, HIGH);
  Serial.println("Rele 1 OFF");
  Serial.println("Rele 2 OFF");
  Rele1On = false;
  Rele2On = false;

  //Sensor
  
  pinMode(DHT1POWERPIN, OUTPUT);
  digitalWrite(DHT1POWERPIN, HIGH);
  pinMode(DHT2POWERPIN, OUTPUT);
  digitalWrite(DHT2POWERPIN, HIGH);
  delay(500);
  dht1.begin();
  dht2.begin();

}

/*--------------LOOP--------------*/
void loop() {    
  //Getting sensor 1 value
  hum1 = dht1.readHumidity();
  temp1 = dht1.readTemperature();
  hum2 = dht2.readHumidity();
  temp2 = dht2.readTemperature();

  if (isnan(temp1)){
    nan1_counter ++;
  }else{
    dht1_fail = false;
    nan1_counter = 0;
  }
  if (isnan(temp2)){
    nan2_counter ++;
  }else{
    dht2_fail = false;
    nan2_counter = 0;
  }
  
  if (nan1_counter >= MAX_NAN && nan2_counter >= MAX_NAN){
    //---------------------------------------------------DHT1 & DHT2 Fail
    digitalWrite(DHT1POWERPIN, !digitalRead(DHT1POWERPIN));
    digitalWrite(DHT2POWERPIN, !digitalRead(DHT2POWERPIN));
    dht1_fail = true;
    dht2_fail = true;
    nan1_counter = 0;
    nan2_counter = 0;
    Serial.print("Both fail\n");
  }else if (nan1_counter >= MAX_NAN){   
    //---------------------------------------------------Only DHT1 Fail
    digitalWrite(DHT1POWERPIN, !digitalRead(DHT1POWERPIN));
    dht1_fail = true;
    nan1_counter = 0;
    Serial.print("DHT1 fail\n");
  }else if (nan2_counter >= MAX_NAN){
    //---------------------------------------------------Only DHT2 Fail
    digitalWrite(DHT2POWERPIN, !digitalRead(DHT2POWERPIN));
    dht2_fail = true;
    nan2_counter = 0;
    Serial.print("DHT2 fail\n");
  }

  if(dht1_fail && dht2_fail){
    //---------------------------------------------------Turn OFF all the Rele
    temp1 = DEFAULT_RELE1_HIGH_TEMP+1;
    temp2 = DEFAULT_RELE2_HIGH_TEMP+1;
    Serial.print("Both Rele OFF\n");
  }else if(dht1_fail){
    temp1 = temp2;
    Serial.print("Rele 1 based on DHT2\n");
  }else if(dht2_fail){
    temp2 = temp1;
    Serial.print("Rele 2 based on DHT1\n");
  }
  
  //Debug write
  //Serial.print("DHT22 1: ");
  //printDebugTempHum(temp1, hum1);
  //Serial.print("DHT22 2: ");
  //printDebugTempHum(temp2, hum2);
  //Manage Rele
  manageRele(1, temp1, DEFAULT_RELE1_LOW_TEMP, DEFAULT_RELE1_HIGH_TEMP);
  manageRele(2, temp2, DEFAULT_RELE2_LOW_TEMP, DEFAULT_RELE2_HIGH_TEMP);
  
  //Monitor write
  lcd.setCursor(0,0);
  typewriting("T1:" + getStringFromFloat(temp1));
  lcd.setCursor(0,1);
  typewriting("H1:" + getStringFromFloat(hum1));
  lcd.setCursor(9,0);
  typewriting("T2:" + getStringFromFloat(temp2));
  lcd.setCursor(9,1);
  typewriting("H2:" + getStringFromFloat(hum2));

  delay(1000);
}

/*--------------FUNCTIONS--------------*/
void typewriting(String messaggio) {
  int lunghezza = messaggio.length();
  for (int i = 0; i < lunghezza; i++) {
    lcd.print(messaggio[i]);
  }
}

String getStringFromFloat(float var){
  return String(var).substring(0, String(var).length() - 1);
}

void printDebugTempHum(float temp, float hum) {
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(" %, Temp: ");
  Serial.print(temp);
  Serial.println(" Celsius");
}

bool manageRele(int rele, float temp, float lowTemp, float highTemp){
  int retVal = true;
  
  if(isnan(temp)){
    retVal = false;
  }else{  
    switch(rele){
      case 1:
          if(temp < lowTemp && Rele1On == false){
            digitalWrite(RELE1PIN, LOW);
            Serial.println("Rele 1 ON");
            Rele1On = true;
          }
          else if(temp > highTemp && Rele1On == true)
          {
            digitalWrite(RELE1PIN, HIGH);
            Serial.println("Rele 1 OFF");
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
            Serial.println("Rele 2 ON");
            Rele2On = true;
          }
          else if(temp > highTemp && Rele2On == true)
          {
            digitalWrite(RELE2PIN, HIGH);
            Serial.println("Rele 2 OFF");
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
  }

  return retVal;
}
