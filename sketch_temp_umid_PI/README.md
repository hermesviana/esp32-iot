# ESP32 IoT
Projeto usando ESP32 para leitura de temperatura e umidade.

## 👤 Autor:
Hermes Viana

## componetes fisicos.
- motor dc 5 volts
- modulo 2 reles 5v
- protoboard 
- display oled 0,96"
- sensor de temperatura e umidade dht11
- placa de desenvolvimento esp32 dev
- cabo usb/c e fios coloridos 

Ligações básicas
Componente	ESP32
DHT11	GPIO (ex: 4)
OLED SDA	21
OLED SCL	22
Relé	GPIO (ex: 5)
Difusor de ar (opcional pin 18)
 
## Como usar
1. Configure o arquivo steak.ino
2. Envie o código para o ESP32
3. Insira login e senha wifi e dopois usuario e chave de conta adafruit.oi
4. Instale as bibliotecas abaixo:

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

“Sistema IoT com controle remoto de setpoint via MQTT, automação de atuadores e monitoramento local e em nuvem.”

🎯 O QUE MUDOU/corrigido (IMPORTANTE)

✔️ OLED inicia antes de tudo
✔️ Delay de estabilização I2C
✔️ MQTT por último
✔️ Reduzido impacto do WiFi
✔️ OLED isolado no setup
✔️ Delay maior pra estabilizar I2C
✔️ Loop mais leve

📱 Controlar temperatura, exemplo:

Ver status e controle slider com botão aplicar:
http://IP_DO_ESP32/

🔥 VISÃO GERAL (O QUE ACONTECE)

ESP32 virou basicamente um termostato com interface web.

Ele faz 4 coisas ao mesmo tempo:

🌡️ lê temperatura/umidade (DHT11)
🌐 recebe setpoint pelo navegador
⚙️ controla o relé automaticamente
📡 envia dados pro Adafruit IO.

🧠 Arquitetura atual

O sistema opera de forma local e independente, com controle rápido e confiável:

📌 Melhorias do Projeto
🔄 Substituição de delay() por millis(), deixando o sistema não bloqueante
🌐 Interface web local com slider para ajuste do setpoint em tempo real
🌡️ Controle automático de temperatura com histerese (liga/desliga estável)
🔌 Controle de relé para acionamento de carga (ex: ventilação/aquecimento)
☁ Integração com Adafruit IO via MQTT para monitoramento remoto
📟 Display OLED com exibição de temperatura, umidade, setpoint e status
🔁 Reconexão automática de WiFi e MQTT para maior estabilidade
⚡ Execução simultânea de Web Server, sensores e IoT sem travamentos.

🚀 Resultado

O projeto se tornou um controlador de temperatura funcional com interface web, pronto para expansão e aplicação em cenários reais de automação.

![Interface](imagens/interface_via_web_slider.png)

![Funcionamento](imagens/controlador_de_temperatura.png)

![AdafruitIO](imagens/dashboads_graficos.png)

