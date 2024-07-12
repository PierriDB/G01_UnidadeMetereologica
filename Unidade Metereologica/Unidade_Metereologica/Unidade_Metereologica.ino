//https://www.youtube.com/watch?v=dwewIsXnW_4

//BME280
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;

//Efeito Hall
#define DETECT_A 2 // PLUVIOMETRO
#define DETECT_B 3 // vELOCIDADE VENTO
#define ACTION 8 // pin 8 for action to do someting

//AS5600
#include "AS5600.h"
AS5600 as5600;   //  use default Wire




void setup() {
  //BME280
  Serial.begin(9600);
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);

  //Efeito Hall
  Serial.println("Hall Module Test");
  pinMode(DETECT_A, INPUT);//define detect input pin
  pinMode(DETECT_B, INPUT);//define detect input pin
  pinMode(ACTION, OUTPUT);//define ACTION output pin
  }

  //AS5600
  Serial.begin(9600);
  Serial.println(__FILE__);
  Serial.print("AS5600_LIB_VERSION: ");
  Serial.println(AS5600_LIB_VERSION);
  Wire.begin();
  as5600.begin(4);  //  set direction pin.
  as5600.setDirection(AS5600_CLOCK_WISE);  //  default, just be explicit.
}

void loop() {
  //BME280
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println("*C");
  Serial.print("Pressure = ");
  Serial.print(bme.readPressure() / 100.0F);
  Serial.println("hPa");
  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println("m");
  Serial.print("Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println("%");


  //Efeito Hall
  int detected_a = digitalRead(DETECT_A);// read Hall sensor
  if( detected_a == LOW)  {
    digitalWrite(ACTION,LOW);// set the buzzer ON
    Serial.println("Detected A!");  
  }
  int detected_b = digitalRead(DETECT_B);// read Hall sensor
  if( detected_b == LOW)  {
    digitalWrite(ACTION,LOW);// set the buzzer ON
    Serial.println("Detected B!");  
  }

  //AS5600 - Encoder
  Serial.print("Angle = ");
  Serial.println(as5600.rawAngle() * AS5600_RAW_TO_RADIANS*180/3.1415);

  Serial.println();
  delay(30000);  
}
