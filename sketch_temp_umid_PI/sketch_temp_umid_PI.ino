#include "secrets.h"

#include <WiFi.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// ===== WIFI =====
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

// ===== ADAFRUIT IO =====
const char* aio_user = AIO_USERNAME;
const char* aio_key = AIO_KEY;
const char* mqtt_server = "io.adafruit.com";

// ===== TELEGRAM =====
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

WiFiClient espClient;
PubSubClient client(espClient);
WebServer server(80);

// ===== DHT =====
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ===== OLED =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== RELÉ =====
#define RELAY_PIN 5
#define RELAY_ACTIVE_LOW true

// ===== VARIÁVEIS =====
float temperatura = 0;
float umidade = 0;
bool motorState = false;

float TEMP_ON = 24.0;
float TEMP_OFF = 22.5;

String ipStr = "";

// ===== TIMERS =====
unsigned long lastSensor = 0;
unsigned long lastMQTT = 0;
unsigned long lastOLED = 0;
unsigned long lastTelegram = 0;

// ===== INTERVALOS =====
const long sensorInterval = 2000;
const long mqttInterval = 5000;
const long oledInterval = 1000;
const long telegramInterval = 30000;

// ===== WIFI =====
void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  WiFi.begin(ssid, password);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 5000) {
    delay(300);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi OK");
    ipStr = WiFi.localIP().toString();
  } else {
    Serial.println("\nOFFLINE");
    ipStr = "";
  }
}

// ===== MQTT =====
void reconnectMQTT() {
  if (client.connected()) return;

  String id = "ESP32_" + String(random(9999));

  if (client.connect(id.c_str(), aio_user, aio_key)) {
    Serial.println("MQTT OK");
  }
}

// ===== RELÉ =====
void setMotor(bool state) {
  motorState = state;
  digitalWrite(RELAY_PIN, (RELAY_ACTIVE_LOW ? !state : state));
}

// ===== WEB =====
String pagina() {
  String html = "<html><body style='text-align:center'>";
  html += "<h2>ESP32 IoT</h2>";
  html += "Temp: " + String(temperatura) + "<br>";
  html += "Hum: " + String(umidade) + "<br>";
  html += "Motor: " + String(motorState ? "ON" : "OFF") + "<br>";
  html += "Set: <span id='sp'>" + String(TEMP_ON) + "</span><br>";

  html += "<input type='range' min='18' max='35' value='" + String(TEMP_ON) + "' oninput='send(this.value)'>";

  html += R"rawliteral(
<script>
function send(v){
  document.getElementById('sp').innerHTML = v;
  fetch('/set?temp='+v);
}
setInterval(()=>{location.reload();},5000);
</script>
)rawliteral";

  html += "</body></html>";
  return html;
}

void handleRoot() {
  server.send(200, "text/html", pagina());
}

void handleSet() {
  if (server.hasArg("temp")) {
    float novo = server.arg("temp").toFloat();
    TEMP_ON = constrain(novo, 18, 35);
    TEMP_OFF = TEMP_ON - 0.5;
  }
  server.send(200, "text/plain", "OK");
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  setMotor(false);

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED FAIL");
    while (true);
  }

  dht.begin();
  connectWiFi();

  secured_client.setInsecure();

  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.begin();

  client.setServer(mqtt_server, 1883);

  bot.sendMessage(CHAT_ID, "ESP32 iniciou", "");
}

// ===== LOOP =====
void loop() {

  server.handleClient();
  connectWiFi();

  if (WiFi.status() == WL_CONNECTED) {
    reconnectMQTT();
    client.loop();
  }

  // SENSOR
  if (millis() - lastSensor > sensorInterval) {
    lastSensor = millis();

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t)) temperatura = t;
    if (!isnan(h)) umidade = h;

    if (temperatura >= TEMP_ON && !motorState) setMotor(true);
    if (temperatura <= TEMP_OFF && motorState) setMotor(false);
  }

  // MQTT
  if (millis() - lastMQTT > mqttInterval) {
    lastMQTT = millis();

    if (client.connected()) {
      client.publish((String(aio_user)+"/feeds/temperatura").c_str(), String(temperatura).c_str());
      client.publish((String(aio_user)+"/feeds/umidade").c_str(), String(umidade).c_str());
    }
  }

  // OLED
  if (millis() - lastOLED > oledInterval) {
    lastOLED = millis();

    display.clearDisplay();

    display.setCursor(0,0);
    display.print("Temp: "); display.print(temperatura);

    display.setCursor(0,10);
    display.print("Hum: "); display.print(umidade);

    display.setCursor(0,20);
    display.print("Motor: "); display.print(motorState ? "ON" : "OFF");

    display.setCursor(0,30);
    display.print("Set: "); display.print(TEMP_ON);

    if (ipStr != "") {
      display.setCursor(0,40);
      display.print("IP: "); display.print(ipStr);
    }

    display.display();
  }

  // TELEGRAM
  if (WiFi.status() == WL_CONNECTED && millis() - lastTelegram > telegramInterval) {
    lastTelegram = millis();

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      for (int i = 0; i < numNewMessages; i++) {

        String text = bot.messages[i].text;
        String chat_id = bot.messages[i].chat_id;

        if (chat_id != CHAT_ID) continue;

        if (text == "/status") {
          bot.sendMessage(chat_id,
            "Temp: " + String(temperatura) +
            "\nHum: " + String(umidade) +
            "\nSet: " + String(TEMP_ON),
            "");
        }
      }
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
  }
}