//https://chatgpt.com/c/67375e93-cee8-8003-ac32-b34c98e4ccdf
#include <WiFi.h>
#include <HTTPClient.h>

// Configurações do Wi-Fi
const char* ssid = "APTO22_5G-EXT";
const char* password = "999953781";

// URL do Google Apps Script (copiada do passo anterior)
const String googleScriptUrl = "https://script.google.com/macros/s/AKfycbxdEHdyuVlrAp2E_af_4uVvu8j77f-aVEW2h-FMUdaV1n73Q_BnHwoGZs8s--WnlCr3/exec";

// Dados dos sensores (exemplo, substitua pelos seus dados reais)
float temperatura = 25.3;  // Substitua com a leitura real do sensor
float umidade = 60.0;      // Substitua com a leitura real do sensor
float angulo = 23.5;       // Substitua com a leitura real do sensor

HTTPClient http;  // Objeto HTTPClient

void setup() {
  // Inicia o monitor serial
  Serial.begin(115200);
  
  // Conecta-se à rede Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando-se ao Wi-Fi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println("\nConectado!");

  // Conecta-se ao Google Apps Script apenas uma vez
  http.begin(googleScriptUrl);  // Abre a conexão
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
}

void loop() {
  // Monta os dados a serem enviados
  String postData = "temperatura=" + String(temperatura) + 
                    "&umidade=" + String(umidade) + 
                    "&angulo=" + String(angulo);
    
  // Envia os dados via POST
  int httpCode = http.POST(postData);
  
  if (httpCode > 0) {
    Serial.println("Dados enviados com sucesso!");
  } else {
    Serial.println("Erro ao enviar dados: " + String(httpCode));
  }

  // Espera 30 segundos antes da próxima leitura
  delay(30000);  // 30 segundos
}