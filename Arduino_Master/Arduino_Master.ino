//https://www.youtube.com/watch?v=dwewIsXnW_4

/******************* Libraries ******************/
//BME280
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "AS5600.h"


/******************* Definitions *******************/
//Efeito Hall
#define hall_pluv 2 // PLUVIOMETRO
#define hall_anem 3 // VELOCIDADE VENTO
#define SEALEVELPRESSURE_HPA (1013.25)
#define address 0x20
//

//AS5600
AS5600 as5600;   //  use default Wire
Adafruit_BME280 bme;

/******************* Variables *******************/
int pressure;
float temperature;
int height;
int humidity;

//Anemometer
int period=10000;
float speedwind=0;
float windspeed=0;
unsigned int RPM=0;
unsigned int counter=0;
const float pi=3.14159265;
int radius=147;
//Pluviometer
int value_pluv = 0;
int old_value_pluv = 0;
int count_pluv = 0;
const float vol_click = 0.25;
//String wind_dir_text[1] = "arduino";

String wind_dir_text= "Arduino Factory"; 

/******************************* Setup ******************************/
void setup() {
  //resetmicros();
  Serial.begin(9600);
  initsensors();
  
  //Efeito Hall
  pinMode(hall_pluv, INPUT);//define detect input pin
  pinMode(hall_anem, INPUT);//define detect input pin

  //AS5600  
  Serial.println(__FILE__);
  Serial.print("AS5600_LIB_VERSION: ");
  Serial.println(AS5600_LIB_VERSION);
  Wire.begin();
  as5600.begin(4);  //  set direction pin.
  as5600.setDirection(AS5600_CLOCK_WISE);  //  default, just be explicit.
}


/****************************** Reading Loop ****************************/
void loop() {
  //BME280
  Serial.print("Temperature = ");
  temperature=(bme.readTemperature());
  Serial.print(temperature);
  Serial.println("*C");
  Serial.print("Pressure = ");
  pressure=bme.readPressure();
  Serial.print(pressure);
  Serial.println(" hPa");
  Serial.print("Approx. Altitude = ");
  height=(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.print(height);
  Serial.println(" m");
  Serial.print("Humidity = ");
  humidity=(bme.readHumidity());
  Serial.print(humidity);
  Serial.println(" %");

  
  //Efeito Hall Pluviômetro
  value_pluv = digitalRead(hall_pluv);
  Serial.print("Reed Pluviômetro: ");
  Serial.println(value_pluv); 
  if(value_pluv!=old_value_pluv)  {  
   count_pluv++;
   old_value_pluv = value_pluv;         
   Serial.print("Medida de chuva (contagem): ");
   Serial.print(count_pluv);
   Serial.println(" pulso");
   Serial.print("Medida de chuva (calculado): ");
   Serial.print(count_pluv * vol_click); 
   Serial.println(" mm");
  } 
  else{
   old_value_pluv = value_pluv;
   Serial.print("Medida de chuva (calculado): ");
   Serial.print(count_pluv * vol_click); 
   Serial.println(" mm");
 }


  //Efeito Hall Anemometer
  windvelocity();
  Serial.print("Counter: ");
  Serial.println(counter);
  RPMcalc();
  WindSpeed();
  Serial.print("Wind speed: ");
  Serial.print(windspeed);
  Serial.println(" [m/s]");              
  SpeedWind();

  //AS5600 - Encoder  
  int wind_dir = int(as5600.rawAngle() * AS5600_RAW_TO_RADIANS*180/3.1415);
  if (wind_dir > 22.5 && wind_dir < 67.5) {
    wind_dir_text = "NE";
  }
  else if (wind_dir > 67.5 && wind_dir < 112.5) {
    wind_dir_text = "N";
  }
  else if (wind_dir > 112.5 && wind_dir < 157.5) {
    wind_dir_text = "NO";
  }
  else if (wind_dir > 157.5 && wind_dir < 202.5) {
    wind_dir_text = "O";
  }
  else if (wind_dir > 202.5 && wind_dir < 247.5) {
    wind_dir_text = "SO";
  }
  else if (wind_dir > 247.5 && wind_dir < 292.5) {
    wind_dir_text = "S";
  }
  else if (wind_dir > 292.5 && wind_dir < 337.5) {
    wind_dir_text = "SE";
  }
  else  {  
    wind_dir_text = "L";
  }
  Serial.print("Angle = ");
  Serial.println(wind_dir);
  Serial.print("Direction = ");
  Serial.println(wind_dir_text);
  Serial.println();
  delay(5000);  
}
/*******************************************************************/
/**************************** Functions ****************************/
/*******************************************************************/

void windvelocity(){
  speedwind = 0;
  windspeed = 0;
  counter = 0;  
  attachInterrupt(digitalPinToInterrupt(hall_anem), addcount, RISING);
  unsigned long millis();       
  long startTime = millis();
  while(millis() < startTime + period) {
  }
}

void addcount(){
  counter++;
} 

void RPMcalc(){
  RPM= counter*(60/(period/1000));  // Calculate revolutions per minute (RPM)
}

void WindSpeed(){
  windspeed = ((2 * pi * radius * RPM)/60);  // Calculate wind speed on m/s
}

void SpeedWind(){
  speedwind = (((2 * pi * radius * RPM)/60))*3.6;  // Calculate wind speed on km/h
}

void resetmicros(){
  Serial.println("Reseting Uno");
  //digitalWrite(reset00, LOW);
  asm volatile ("jmp 0");
  //wdt_enable(WDTO_15MS);  // Define o timer para o menor tempo possível (15 ms)
  while (1); // Aguarda o reset do Watchdog Timer
}

void initsensors() {
//init BME280
Serial.begin(9600);
if (!bme.begin(0x76)) {
  Serial.println("Could not find a valid BME280 sensor, check wiring!");
  Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
  Serial.print(" ID of 0x60 represents a BME 280.\n");
  while (1);
  }
}

