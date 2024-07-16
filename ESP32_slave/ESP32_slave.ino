// Adafruit IO Digital Output Example
// Tutorial Link: https://learn.adafruit.com/adafruit-io-basics-digital-output
//
// Adafruit invests time and resources providing this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit!
//
// Written by Todd Treece for Adafruit Industries
// Copyright (c) 2016 Adafruit Industries
// Licensed under the MIT license.
//
// All text above must be included in any redistribution.

/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"
#include <Wire.h> // Biblioteca nativa do core Arduino


/************************ Example Starts Here *******************************/

// digital pin 5
// Variáveis globais
const int myAddress = 0x10; // armazena o endereço deste dispositivo (slave)
#define LED_PIN 2
int t;
float tempValue = 25.5;


// set up the 'digital' feed
AdafruitIO_Feed *digital = io.feed("digital");
AdafruitIO_Feed *Altitude = io.feed("Altitude");
AdafruitIO_Feed *Pluviometria = io.feed("Pluviometria");
AdafruitIO_Feed *Pressao = io.feed("Pressao");
AdafruitIO_Feed *Umidade = io.feed("Umidade");
AdafruitIO_Feed *temperatura = io.feed("temperatura");
AdafruitIO_Feed *Vento_velocidade = io.feed("Vento_velocidade");
AdafruitIO_Feed *Vento_direcao = io.feed("Vento_direcao");



void setup() {
  
  pinMode(LED_PIN, OUTPUT);  
  Serial.begin(115200);
  // wait for serial monitor to open
  while(! Serial);

  // connect to io.adafruit.com
  Serial.print("Connecting to Adafruit IO");
  io.connect();

  digital->onMessage(handleMessage); //Quando receber um sinal do adafruit, chama a função

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  digital->get();

  //Conexão Serial com Arduin
  //Serial.begin(115200);  // Configura a taxa de transferência em bits por segundo (baud rate) para transmissão serial.
  Serial.println();
  Wire.begin(myAddress); // inicia o dispositivo com o endereço definido anteriormente
  Wire.onReceive(receiveEvent); //registra o evento de recebimento de mensagem
  Serial.println("Software do ESP32");

}

void loop() {
  // io.run(); is required for all sketches.
  // it should always be present at the top of your loop function. it keeps the client connected to io.adafruit.com, and processes any incoming data.
  io.run();
  Serial.println(tempValue);
  tempValue = tempValue + 0.1;
  temperatura -> save(tempValue);
  Umidade  -> save(tempValue);
  Altitude -> save(tempValue);
  Pressao -> save(tempValue);
  Vento_velocidade -> save(tempValue);
  Vento_direcao -> save("NE");
  Pluviometria -> save(tempValue);
  delay(20000);
}

// this function is called whenever an 'digital' feed message
// is received from Adafruit IO. it was attached to the 'digital' feed in the setup() function above.
void handleMessage(AdafruitIO_Data *data) {
  Serial.print("received <- ");
  if(data->toPinLevel() == HIGH)
    Serial.println("HIGH");
  else
    Serial.println("LOW");
  digitalWrite(LED_PIN, data->toPinLevel());
}


void receiveEvent(int howMany) {
  String message = readString();
  Serial.print(message); // imprime a mensagem recebida
}

String readString() {
  String retorno;  
  while (Wire.available()) { // Enquanto houver bytes disponíveis para leitura, ...
    char c = Wire.read(); // recebe o byte como caractere
    retorno += c;
  }
  return retorno;
}
