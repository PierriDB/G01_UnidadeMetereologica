/******************************************************************************
                    Comunicação Mestre e Escravo - Sketch para MASTER
                                 Sketch de Exemplo

                         Criado em 12 de Agosto de 2022
                               por Michel Galvão

  Placa: Arduino Uno
  O sketch usa 4170 bytes (12%) de espaço de armazenamento para programas. 
    O máximo são 32256 bytes.
  Variáveis globais usam 257 bytes (12%) de memória dinâmica, deixando 1791 bytes 
    para variáveis locais. O máximo são 2048 bytes.


  Blog Eletrogate - Veja este e outros projetos e tutoriais no blog Eletrogate
                            https://blog.eletrogate.com/

  Eletrogate - Loja de Arduino \\ Robótica \\ Automação \\ Apostilas \\ Kits
                            https://www.eletrogate.com/
******************************************************************************/

// Inclusão da biblioteca
#include <Wire.h> // Biblioteca nativa do core Arduino

// Variáveis globais
const int slaveAddress = 0x08; // constante armazena o endereço do dispositivo slave
int x = 0; // variável de armazenagem do valor para envio

void setup() {
  Wire.begin(); // inicia a comunicação I2C
}

void loop() {
  Wire.beginTransmission(slaveAddress); // transmite para o dispositivo slave

  // Cria e formata a mensagem para envio
  String message = "O valor de x é ";
  message.concat(x);
  message.concat("\n");

  Wire.write(message.c_str());// envia a mensagem
  Wire.endTransmission();  //para de transmitir

  x++; // incremento da variável
  delay(500); // pausa de 500 milissegundos
}