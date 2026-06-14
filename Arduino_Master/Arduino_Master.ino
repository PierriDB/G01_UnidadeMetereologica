#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "AS5600.h"

#define hall_pluv 5
#define hall_anem 3

#define SEALEVELPRESSURE_HPA (1013.25)

AS5600 as5600;
Adafruit_BME280 bme;

unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 30000;

int count_pluv = 0;
int old_value_pluv = 0;

float vol_click = 6173;
float A_cil = 9503;

int period = 10000;
float windspeed = 0;
unsigned int counter = 0;

const float pi = 3.14159265;
int radius = 85;

String wind_dir_text = "L";

void setup() {
  Serial.begin(9600); // 🔥 IMPORTANTE

  Wire.begin();

  pinMode(hall_pluv, INPUT);
  pinMode(hall_anem, INPUT);

  as5600.begin(4);
  as5600.setDirection(AS5600_CLOCK_WISE);

  if (!bme.begin(0x76)) {
    while (1);
  }

  attachInterrupt(digitalPinToInterrupt(hall_anem), addcount, RISING);
}

void loop() {

  if (millis() - lastUpdateTime >= updateInterval) {
    lastUpdateTime = millis();

    float temperature = bme.readTemperature();
    float pressure = bme.readPressure() / 100.0; // 🔥 corrigido
    float height = bme.readAltitude(SEALEVELPRESSURE_HPA);
    float humidity = bme.readHumidity();

    int value_pluv = digitalRead(hall_pluv);
    if (value_pluv != old_value_pluv) {
      count_pluv++;
      old_value_pluv = value_pluv;
    }

    windvelocity();
    WindSpeed();

    int wind_dir = (int)(as5600.rawAngle() * 180.0 / 3.1415);

    if (wind_dir > 337 || wind_dir < 22) wind_dir_text = "L";
    else if (wind_dir < 67) wind_dir_text = "NE";
    else if (wind_dir < 112) wind_dir_text = "N";
    else if (wind_dir < 157) wind_dir_text = "NO";
    else if (wind_dir < 202) wind_dir_text = "O";
    else if (wind_dir < 247) wind_dir_text = "SO";
    else if (wind_dir < 292) wind_dir_text = "S";
    else wind_dir_text = "SE";

    String data =
      String(temperature) + "," +
      String(pressure) + "," +
      String(height) + "," +
      String(humidity) + "," +
      String(count_pluv * vol_click / A_cil) + "," +
      String(windspeed) + "," +
      String(wind_dir) + "," +
      wind_dir_text;

    Serial.println(data); // 🔥 envia LIMPO
  }
}

void addcount() {
  counter++;
}

void windvelocity() {
  counter = 0;
  unsigned long startTime = millis();
  while (millis() < startTime + period) {}
}

void WindSpeed() {
  windspeed = ((2 * pi * radius * counter) / period) * 3.6;
}