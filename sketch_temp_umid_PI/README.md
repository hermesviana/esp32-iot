# ESP32 IoT
Projeto usando ESP32 para leitura de temperatura e umidade.

## componetes fisicos.
- motor dc
- modulo 2 reles 5v
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
sensor dht11 pin 4
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
🔌 Controle remoto de motor (via interface web)
📱 Acesso via dashboard no celular ou navegador

📌 Evolução do Projeto

Este projeto evoluiu de um sistema básico de leitura de sensores para um controlador de temperatura completo com interface web e arquitetura IoT organizada.

🔧 Melhorias implementadas
Implementação de controle automático de temperatura com histerese, evitando acionamentos instáveis do relé
Criação de interface web em tempo real, permitindo monitoramento e ajuste do sistema via navegador
Adição de controle de setpoint por slider, eliminando necessidade de comandos manuais por URL
Integração com display OLED, exibindo temperatura, umidade, estado do sistema e setpoint
Implementação de servidor web embarcado no ESP32 para comunicação local sem dependência externa
Integração com MQTT (Adafruit IO) para envio de dados de monitoramento
Separação de responsabilidades:
🌐 Web → controle
📡 MQTT → monitoramento
🔐 Segurança e organização
Remoção de credenciais do código principal.

🧠 Arquitetura atual

O sistema opera de forma local e independente, com controle rápido e confiável:

O usuário ajusta o setpoint via interface web
O ESP32 processa a lógica de controle
O relé é acionado automaticamente conforme a temperatura
O OLED fornece feedback em tempo real
O MQTT envia dados para monitoramento remoto
🚀 Resultado

O projeto se tornou um controlador de temperatura funcional com interface web, pronto para expansão e aplicação em cenários reais de automação.
