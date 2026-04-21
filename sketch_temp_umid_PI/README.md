# ESP32 IoT

Projeto usando ESP32 para leitura de temperatura e umidade.

## Arquivos
- sketch_temp_umid_PI.ino
- secrets.h
- gitignore

## componetes fisicos.
- motor dc
- modulo rele 5v
- protoboard 
- display oled 0,96"
- sensor de temperatura e umidade dht11
- placa de desenvolvimento esp32 dev
- cabo usb c e fios coloridos 

Ligações básicas
Componente	ESP32
DHT	GPIO (ex: 4)
OLED SDA	21
OLED SCL	22
Relé	GPIO (ex: 5)
- 

## Como usar
1. Configure o arquivo steak.ino
2. Envie o código para o ESP32
3. Insira login e senha wifi e dopois usuario e chave de adafruit.oi
4. Instale as bibliotecas abaixo 
#include <WiFi.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Funcionalidades
📊 Leitura de temperatura e umidade (sensor DHT11/DHT22)
🖥️ Exibição em display OLED 0.96"
☁️ Envio de dados para o Adafruit IO (MQTT)
🔌 Controle remoto de motor (via feed)
📱 Acesso via dashboard no celular ou navegador
