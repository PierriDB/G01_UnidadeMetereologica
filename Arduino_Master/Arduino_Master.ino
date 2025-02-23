//https://www.youtube.com/watch?v=dwewIsXnW_4

/******************* Libraries ******************/
//BME280
#include <Wire.h>
#include <A2a.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "AS5600.h"


/******************* Definitions *******************/
//Efeito Hall
#define hall_pluv 5 // PLUVIOMETRO
#define hall_anem 3 // VELOCIDADE VENTO
#define SEALEVELPRESSURE_HPA (1013.25)
#define endereco 0x20
//

//AS5600
AS5600 as5600;   //  use default Wire
A2a ESP32;
Adafruit_BME280 bme;

/******************* Variables *******************/
const int slaveAddress = 0x10; // constante armazena o endereço do dispositivo slave
int x = 0; // variável de armazenagem do valor para envi

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
int radius=85;
//Pluviometer
int value_pluv = 0;
int old_value_pluv = 0;
int count_pluv = 0;
float vol_click = 6173; //mm3 1L deu 162 batidas na gangorra --> 6.173 cm3
float A_cil = 9503;   //mm2 sendo pi*55^2 --> 95.03 cm2
static unsigned long lastUpdateTime = 0; // Armazena o tempo da última atualização
const unsigned long updateInterval = 30000; // Intervalo de tempo em milissegundos (20 segundos) + 10seg do resto
String wind_dir_text= "Arduino Factory"; 

/******************************* Setup ******************************/
void setup() {
  Wire.begin(); // inicia a comunicação I2C
  ESP32.begin(endereco);

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

  // Verifica se o intervalo de tempo passou
  if (millis() - lastUpdateTime >= updateInterval) {
    lastUpdateTime = millis(); // Atualiza o tempo da última execução

    //BME280
    temperature = bme.readTemperature();
    pressure = bme.readPressure();
    height = bme.readAltitude(SEALEVELPRESSURE_HPA);
    humidity = bme.readHumidity();

    // Efeito Hall Pluviômetro
    value_pluv = digitalRead(hall_pluv);
    if (value_pluv != old_value_pluv) {  
      count_pluv++;
      old_value_pluv = value_pluv;         
    } else {
      old_value_pluv = value_pluv;
    }

    // Efeito Hall Anemometer
    windvelocity();
    WindSpeed();

    // AS5600 - Encoder  
    int wind_dir = int(as5600.rawAngle() * AS5600_RAW_TO_RADIANS * 180 / 3.1415);
    if (wind_dir > 22.5 && wind_dir < 67.5) {
      wind_dir_text = "NE";
    } else if (wind_dir > 67.5 && wind_dir < 112.5) {
      wind_dir_text = "N";
    } else if (wind_dir > 112.5 && wind_dir < 157.5) {
      wind_dir_text = "NO";
    } else if (wind_dir > 157.5 && wind_dir < 202.5) {
      wind_dir_text = "O";
    } else if (wind_dir > 202.5 && wind_dir < 247.5) {
      wind_dir_text = "SO";
    } else if (wind_dir > 247.5 && wind_dir < 292.5) {
      wind_dir_text = "S";
    } else if (wind_dir > 292.5 && wind_dir < 337.5) {
      wind_dir_text = "SE";
    } else {
      wind_dir_text = "L";
    }

    // Prepara e envia os dados
    String data = String(temperature) + "," + 
                  String(pressure) + "," + 
                  String(height) + "," + 
                  String(humidity) + "," + 
                  String(count_pluv * vol_click / A_cil) + "," + 
                  String(windspeed) + "," + 
                  String(wind_dir) + "," + 
                  wind_dir_text;

    Serial.println(data); // Envia a string para o ESP32
    x = x + 1;
  }
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

void WindSpeed(){
  windspeed = ((2 * pi * radius * counter)/period)*3.6;  // Calculate wind speed on km/h
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

