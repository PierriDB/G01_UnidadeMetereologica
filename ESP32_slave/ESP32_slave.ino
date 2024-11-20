
/************************** Configuration ***********************************/
// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"
#include <Wire.h> // Biblioteca nativa do core Arduino
#include "esp_task_wdt.h"
//#include <A2a.h>

/************************ Example Starts Here *******************************/

// digital pin 5
// Variáveis globais
#define LED_PIN 2
#define endereco 0x20 // armazena o endereço deste dispositivo (slave)
#define RXp2 16
#define TXp2 17

int t;
float tempValue = 25.5;

/******************************** Objects ***********************************/
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

  //Define a Serial 2 que recebera os dados do Arduino
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);

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

}

void loop() {
  io.run();

  // Verifica se há dados disponíveis na Serial 2
  if (Serial2.available() > 0) {
    String receivedData = Serial2.readString();  // Lê a string recebida uma única vez
    Serial.println(receivedData);  // Exibe a string recebida
    
    // Prepara e envia os dados para o Adafruit IO
    prepareDataToSend(receivedData);
  }
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


void prepareDataToSend(String string_data) {
  char data[100];  // Define um buffer para armazenar a string convertida
  string_data.toCharArray(data, 100);  // Converte a String para um array de char

  char* tokens[8];  // Array para armazenar as substrings
  int index = 0;
  
  // Usa strtok para separar a string
  char* token = strtok(data, ",");
  while (token != NULL && index < 8) {
    tokens[index] = token;  // Armazena cada substring no array
    index++;
    token = strtok(NULL, ",");
  }

  // Verifica se temos dados válidos antes de tentar salvar
  if (index == 8) {
    // Converte e salva os valores para o Adafruit IO, com verificações adicionais para garantir que as operações sejam seguras
    //Serial.print(atof(tokens[5]));

    temperatura->save(atof(tokens[0]));  // Converte para float
    Pressao->save(atof(tokens[1]));
    Altitude->save(atof(tokens[2]));
    Umidade->save(atof(tokens[3]));
    Pluviometria->save(atof(tokens[4]));
    Vento_velocidade->save(atof(tokens[5]));
    Vento_direcao->save(tokens[7]);  // Não precisa de conversão para texto
    Serial.println("Dados enviados");

  } else {
    Serial.println("Erro: Dados recebidos estão incompletos.");
  }
}

