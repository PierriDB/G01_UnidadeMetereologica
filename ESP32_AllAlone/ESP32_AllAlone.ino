// Arduino_Master_ESP32.ino
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "AS5600.h"
#include "config.h"
#include <WiFi.h>

#define LED_PIN 2
#define hall_pluv 16
#define hall_anem 17
#define SDA_PIN 21
#define SCL_PIN 22
#define SEALEVELPRESSURE_HPA (1013.25)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

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

//_________________________________________________________________________//

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

AS5600 as5600;
Adafruit_BME280 bme;

unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 30000;

#define NUM_MEDIDAS 2880         // Número de medições 24h considerando 30 segundos
int buffer[NUM_MEDIDAS];       // Array para armazenar medições de Pluviometria
int indice = 0;                  // Índice do buffer circular
float somaPluviometria = 0;      // Soma móvel das últimas 2880 medições
int count_pluv = 0;
int old_value_pluv = 0;
float vol_click = 6173;
float A_cil = 9503;

const int period = 10000;
float windspeed = 0;
volatile unsigned int counter = 0;
const float pi = 3.14159265;
int radius = 85;
String wind_dir_text = "L";

void addcount() {
  counter++;
}

void updateDisplay(float t, float h, float w, String dir, bool enviando = false){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0,0);
  display.print("METEOROLOGIA"); 

  // --- Canto Superior Direito: Status de Conexão ---
  display.setCursor(105, 0);
  if (io.status() == AIO_CONNECTED) {
    display.print("IO");
  } else {
    display.print("--");
  }

  // --- Canto Superior Direito: Piscar ao enviar ---
  if (enviando) {
    display.setCursor(120, 0);
    display.print("*"); // Aparece um asterisco piscando quando transmite
  }

  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
  display.setCursor(0, 18);
  display.print("Temp:"); display.print(t, 1); display.print(" C");
  display.setCursor(0, 30);
  display.print("Umid:"); display.print(h, 0); display.print("%");
  display.setCursor(0, 42);
  display.print("Vento:"); display.print(w, 1); display.print("m/s "); display.print(dir);
  display.setCursor(0, 54);
  display.print("Direcao:"); display.print(dir);
  display.display();
}

void windvelocity(){
  counter=0;
  unsigned long start=millis();
  while(millis()-start<period){
    delay(1);
  }
}

void WindSpeed(){
  windspeed=((2*pi*radius*counter)/period)*3.6;
}

void conectarIO() {
  Serial.println("Conectando IO");
  // Mostra na tela que está tentando conectar
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Conectando WiFi/IO");
  display.display();
  io.connect();

  int t = 0;
  while (io.status() < AIO_CONNECTED && t < 40) {
    Serial.print(".");
    display.print("."); // Imprime os pontinhos na tela OLED também
    display.display();
    delay(500);
    t++;
  }

  Serial.println();

  if (io.status() == AIO_CONNECTED) {
    Serial.println("Conectado!");
    display.println();
    display.println("Conectado ao Adafruit");
    display.display();
    delay(1000); // Dá 1 segundo para você conseguir ler a mensagem
  }
}

void atualizarBuffer(int novaMedicao) {  
  buffer[indice] = novaMedicao;// Atualiza o buffer com a contagem total atual de cliques
  int primeiroIndice = (indice + 1) % NUM_MEDIDAS; // Pega o clique de 24h atrás
  
  // A diferença entre o total de agora e o total de 24h atrás dá a quantidade de cliques das últimas 24h.
  // Multiplicamos pelo volume do clique e dividimos pela área para ter a precipitação.
  somaPluviometria = (buffer[indice] - buffer[primeiroIndice]) * vol_click / A_cil;  
  somaPluviometria = round(somaPluviometria * 100) / 100.0; //Arredonda para 2 casas decimais
  
  indice = (indice + 1) % NUM_MEDIDAS; // Atualiza o índice do próximo valor a ser sobrescrito
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  delay(1000);

  // 1. Iniciamos o I2C e a tela OLED PRIMEIRO para poder ver os logs
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println("Erro OLED");
    while(1);
  }
  
  // Prepara a tela inicial
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Iniciando Sistema...");
  display.display();
  delay(1000);

  // 2. Conecta ao Adafruit (OLED vai atualizar os pontinhos dentro da função conectarIO)
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  conectarIO();

  // Prepara a tela para listar os sensores
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Iniciando Sensores:");
  display.display();
  delay(500);

  // 3. Inicia o AS5600 e avisa na tela
  as5600.begin(4);
  as5600.setDirection(AS5600_CLOCK_WISE);
  display.setCursor(0, 18);
  display.println("- AS5600 conectado");
  display.display();
  delay(500);

  // 4. Inicia o BME280 e avisa na tela
  if(!bme.begin(0x76)){
    Serial.println("Erro BME280");
    display.println("- ERRO BME280!");
    display.display();
    while(1);
  }
  display.println("- BME conectado");
  display.display();

  // Restante das configurações de pinos e interrupções
  pinMode(hall_pluv, INPUT_PULLUP);
  pinMode(hall_anem, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(hall_anem), addcount, RISING);

  // Inicializa o buffer pluviometria com zeros
  for (int i = 0; i < NUM_MEDIDAS; i++) {
    buffer[i] = 0;
  }
}

//=====================================================================================//
//=====================================================================================//
//=====================================================================================//

void loop() { // <---- O ERRO ESTAVA AQUI! Adicionado o 'void'
  io.run();

  // Gerencia a conexão e o LED físico
  if (io.status() != AIO_CONNECTED) {
    digitalWrite(LED_PIN, LOW);
    conectarIO(); // Se cair, ele vai mostrar a tela de reconexão automaticamente!
  } else {
    digitalWrite(LED_PIN, HIGH);
  }

  if(millis() - lastUpdateTime >= updateInterval) {
    lastUpdateTime = millis();

    //BME280
    float temperature = bme.readTemperature();
    float pressure = bme.readPressure() / 100.0;
    float height = bme.readAltitude(SEALEVELPRESSURE_HPA);
    float humidity = bme.readHumidity();

    //Pluviometria
    int value_pluv = digitalRead(hall_pluv);
    if(value_pluv != old_value_pluv){
      count_pluv++;
      old_value_pluv = value_pluv;
    }

    // O correto é passar o count_pluv (total de cliques) para o buffer
    atualizarBuffer(count_pluv*A_cil/vol_click); 

    //Anemometro e direção
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

    //  Atualiza a tela com o asterisco LIGADO (começou a enviar)
    updateDisplay(temperature, humidity, windspeed, wind_dir_text, true);

    String data =
      String(temperature)+","+
      String(pressure)+","+
      String(height)+","+
      String(humidity)+","+
      String(count_pluv*vol_click/A_cil)+","+
      String(windspeed)+","+
      String(wind_dir)+","+
      wind_dir_text;
    Serial.println(data);

    // Mandar para Adafruit
    float temp = temperature;
    float press = pressure;
    float alt = height;
    float umid = humidity;
    float pluv = (count_pluv*vol_click/A_cil);
    float vento_v = windspeed;
    const char* vento_d = wind_dir_text.c_str();

    temperatura->save(temp);
    Pressao->save(press);
    Altitude->save(alt);
    Umidade->save(umid);
    Pluviometria->save(pluv);
    
    // Adicionei esta linha para mandar a soma móvel de 24h para o Adafruit também!
    Pluviometria_soma->save(somaPluviometria); 

    Vento_velocidade->save(vento_v);
    Vento_direcao->save(vento_d);

    Serial.println("OK enviado");
    
    // Pisca o LED físico da placa como aviso de envio bem sucedido
    digitalWrite(LED_PIN, LOW);
    delay(200); 
    digitalWrite(LED_PIN, HIGH);

    // 2. Atualiza a tela com o asterisco DESLIGADO (terminou)
    updateDisplay(temperature, humidity, windspeed, wind_dir_text, false);
  }
}