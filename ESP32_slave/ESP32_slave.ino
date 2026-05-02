#include "config.h"
#include <WiFi.h>

#define LED_PIN 2
#define RXp2 16
#define TXp2 17

unsigned long lastSend = 0;

// Feeds
AdafruitIO_Feed *temperatura = io.feed("temperatura");
AdafruitIO_Feed *Pressao = io.feed("Pressao");
AdafruitIO_Feed *Altitude = io.feed("Altitude");
AdafruitIO_Feed *Umidade = io.feed("Umidade");
AdafruitIO_Feed *Pluviometria = io.feed("Pluviometria");
AdafruitIO_Feed *Pluviometria_soma = io.feed("Pluviometria_soma");
AdafruitIO_Feed *Vento_velocidade = io.feed("Vento_velocidade");
AdafruitIO_Feed *Vento_direcao = io.feed("Vento_direcao");

float vol_click = 6173;
float A_cil = 9503;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);

  Serial2.begin(115200, SERIAL_8N1, RXp2, TXp2); // 🔥 IGUAL ARDUINO

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

  conectarIO();
}

void loop() {
  io.run();

  if (io.status() != AIO_CONNECTED) {
    digitalWrite(LED_PIN, LOW);
    conectarIO();
  } else {
    digitalWrite(LED_PIN, HIGH);
  }

  if (Serial2.available()) {

    String line = Serial2.readStringUntil('\n');

    line.replace("\r", "");
    line.trim();

    Serial.println(line);

    if (line.length() < 10) return;

    processar(line);
  }
}

void conectarIO() {
  Serial.print("Conectando IO");

  io.connect();

  int t = 0;
  while (io.status() < AIO_CONNECTED && t < 40) {
    Serial.print(".");
    delay(500);
    t++;
  }

  Serial.println();

  if (io.status() == AIO_CONNECTED) {
    Serial.println("Conectado!");
  }
}

void processar(String s) {

  char buf[120];
  s.toCharArray(buf, 120);

  char* tks[8];
  int i = 0;

  char* tk = strtok(buf, ",");

  while (tk && i < 8) {
    tks[i++] = tk;
    tk = strtok(NULL, ",");
  }

  if (i != 8) {
    Serial.println("Erro dados");
    return;
  }

  if (millis() - lastSend < 5000) return;
  lastSend = millis();

  float temp = atof(tks[0]);
  float press = atof(tks[1]);
  float alt = atof(tks[2]);
  float umid = atof(tks[3]);
  float pluv = atof(tks[4]);
  float vento_v = atof(tks[5]);
  char* vento_d = tks[7];

  temperatura->save(temp);
  Pressao->save(press);
  Altitude->save(alt);
  Umidade->save(umid);
  Pluviometria->save(pluv);
  Vento_velocidade->save(vento_v);
  Vento_direcao->save(vento_d);

  Serial.println("OK enviado");
}

void handleMessage(AdafruitIO_Data *data) {
  if (data->toPinLevel() == HIGH) {
    ESP.restart();
  }
}