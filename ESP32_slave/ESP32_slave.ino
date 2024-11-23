/************************** Configuration ***********************************/
// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"
#include <Wire.h> // Biblioteca nativa do core Arduino
#include "esp_task_wdt.h"

/************************ Example Starts Here *******************************/

// digital pin 5
// Variáveis globais
#define LED_PIN 2
#define endereco 0x20 // armazena o endereço deste dispositivo (slave)
#define RXp2 16
#define TXp2 17


// Configuração para soma móvel
#define NUM_MEDIDAS 2880         // Número de medições 24h considerando 30 segundos
int buffer[NUM_MEDIDAS];       // Array para armazenar medições de Pluviometria
int indice = 0;                  // Índice do buffer circular
float somaPluviometria = 0;      // Soma móvel das últimas 2500 medições
float vol_click = 6173; //mm3 1L deu 162 batidas na gangorra --> 6.173 cm3
float A_cil = 9503;   //mm2 sendo pi*55^2 --> 95.03 cm2
int t;

/******************************** Objects ***********************************/
// set up the 'digital' feed
AdafruitIO_Feed *digital = io.feed("digital");
AdafruitIO_Feed *Altitude = io.feed("Altitude");
AdafruitIO_Feed *Pluviometria = io.feed("Pluviometria");
AdafruitIO_Feed *Pluviometria_soma = io.feed("Pluviometria_soma");
AdafruitIO_Feed *Pressao = io.feed("Pressao");
AdafruitIO_Feed *Umidade = io.feed("Umidade");
AdafruitIO_Feed *temperatura = io.feed("temperatura");
AdafruitIO_Feed *Vento_velocidade = io.feed("Vento_velocidade");
AdafruitIO_Feed *Vento_direcao = io.feed("Vento_direcao");

void setup() {
  pinMode(LED_PIN, OUTPUT);  
  Serial.begin(115200);

  //Define a Serial 2 que receberá os dados do Arduino
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);

  // wait for serial monitor to open
  while (!Serial);

  // connect to io.adafruit.com
  Serial.print("Connecting to Adafruit IO");
  io.connect();
  digital->onMessage(handleMessage); // Quando receber um sinal do Adafruit, chama a função

  // wait for a connection
  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // Indica visualmente que a conexão foi estabelecida
  Serial.println();
  Serial.println(io.statusText());
  digital->get();
  digitalWrite(LED_PIN, HIGH);

  // Inicializa o buffer com zeros
  for (int i = 0; i < NUM_MEDIDAS; i++) {
    buffer[i] = 0;
  }
}

/*********************************************************************/
/***************************** Loop **********************************/
/*********************************************************************/

void loop() {
  io.run();

  // Verifica se há dados disponíveis na Serial 2
  if (Serial2.available() > 0) {
    String receivedData = Serial2.readString();  // Lê a string recebida uma única vez
    Serial.println(receivedData);               // Exibe a string recebida

    // Prepara e envia os dados para o Adafruit IO
    prepareDataToSend(receivedData);
  }
}

/*********************************************************************/
/**************************** Functions ******************************/
/*********************************************************************/

// Atualiza o buffer circular e calcula a diferença entre o último e o primeiro valor
void atualizarBuffer(int novaMedicao) {  
  buffer[indice] = novaMedicao;// Atualiza o buffer com o novo valor
  int primeiroIndice = (indice + 1) % NUM_MEDIDAS; // Primeiro valor no buffer
  somaPluviometria = (buffer[indice] - buffer[primeiroIndice])* vol_click / A_cil;  // Calcula a diferença entre o último e o primeiro valor 
  somaPluviometria = round(somaPluviometria * 100) / 100.0; //Arredonda para 2 casas decimais
  indice = (indice + 1) % NUM_MEDIDAS; // Atualiza o índice do próximo valor a ser sobrescrito
}

//Realiza Reset quando recebe sinal do Adafruit
void handleMessage(AdafruitIO_Data *data) {
  Serial.print("received <- ");
  if (data->toPinLevel() == HIGH) {
    Serial.println("HIGH");

    // Pisca o LED 3 vezes antes do reset
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      delay(200);
    }
    
    // Reseta o ESP32
    ESP.restart();
  } 
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
    // Obtém o valor de Pluviometria
    float valorPluviometria = atof(tokens[4]);  // Converte para float
    
    // Atualiza o buffer e a soma móvel
    atualizarBuffer(valorPluviometria*A_cil/vol_click); //Sai valor de somaPluviometria
    
    // Envia os dados processados para o Adafruit IO
    temperatura->save(atof(tokens[0]));
    Pressao->save(atof(tokens[1]));
    Altitude->save(atof(tokens[2]));
    Umidade->save(atof(tokens[3]));
    Pluviometria->save(valorPluviometria);
    Pluviometria_soma->save(somaPluviometria);
    Vento_velocidade->save(atof(tokens[5]));
    Vento_direcao->save(tokens[7]);

  } else {
    Serial.println("Erro: Dados recebidos estão incompletos.");
  }
}
