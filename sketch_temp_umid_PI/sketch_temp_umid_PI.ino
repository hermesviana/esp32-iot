#include "secrets.h"
#include <WiFi.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===== WIFI =====
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

// ===== ADAFRUIT IO =====
const char* aio_user = AIO_USERNAME;
const char* aio_key = AIO_KEY;

const char* mqtt_server = "io.adafruit.com";

WiFiClient espClient;
PubSubClient client(espClient);
WebServer server(80);

// ===== OLED =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== DHT =====
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ===== RELÉ =====
#define RELAY_PIN 5
#define RELAY_ACTIVE_LOW true

// ===== VARIÁVEIS =====
float temperatura = 0;
float umidade = 0;
bool motorState = false;

// ===== SETPOINT =====
float TEMP_ON = 24.0;
float TEMP_OFF = 22.5;

// ================= WIFI =================
void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  WiFi.begin(ssid, password);
  Serial.print("WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("\nWiFi OK");
  Serial.println(WiFi.localIP());
}

// ================= MOTOR =================
void setMotor(bool state) {
  motorState = state;

  if (RELAY_ACTIVE_LOW) {
    digitalWrite(RELAY_PIN, state ? LOW : HIGH);
  } else {
    digitalWrite(RELAY_PIN, state ? HIGH : LOW);
  }
}

// ================= HTML =================
String paginaHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body {font-family:Arial;background:#111;color:white;text-align:center;}
.card {background:#222;margin:10px;padding:15px;border-radius:12px;}
.value {font-size:30px;color:#00ffcc;}
button {padding:10px 20px;font-size:18px;}
</style>
</head>
<body>

<h2>ESP32 Controle</h2>

<div class="card">
Temp: <div id="t" class="value">--</div>
Hum: <div id="h" class="value">--</div>
Motor: <div id="m" class="value">--</div>
Setpoint: <div id="sp" class="value">--</div>
</div>

<div class="card">
<input type="range" min="18" max="35" value="24" id="slider">
<br><br>
<button onclick="enviar()">Aplicar</button>
</div>

<script>
function enviar() {
  let v = document.getElementById("slider").value;
  fetch("/set?temp=" + v);
}

setInterval(()=>{
fetch('/data')
.then(r=>r.json())
.then(d=>{
t.innerHTML=d.temp+" C";
h.innerHTML=d.hum+" %";
m.innerHTML=d.motor;
sp.innerHTML=d.set;
});
},1000);
</script>

</body>
</html>
)rawliteral";
}

// ================= JSON =================
void handleData() {
  String json = "{";
  json += "\"temp\":" + String(temperatura) + ",";
  json += "\"hum\":" + String(umidade) + ",";
  json += "\"motor\":\"" + String(motorState ? "ON" : "OFF") + "\",";
  json += "\"set\":" + String(TEMP_ON) + ",";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

// ================= WEB =================
void setupWeb() {

  server.on("/", []() {
    server.send(200, "text/html", paginaHTML());
  });

  server.on("/data", handleData);

  server.on("/set", []() {

    if (server.hasArg("temp")) {

      float novo = server.arg("temp").toFloat();

      TEMP_ON = novo;
      TEMP_OFF = novo - 1.5;

      Serial.print("SETPOINT WEB: ");
      Serial.println(TEMP_ON);

      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "ERRO");
    }
  });

  server.begin();
}

// ================= MQTT =================
void reconnectMQTT() {
  if (client.connected()) return;

  String clientId = "ESP32_" + String(random(1000,9999));

  if (client.connect(clientId.c_str(), AIO_USERNAME, AIO_KEY)) {
    Serial.println("MQTT OK");
  }
}

// ================= SETUP =================
void setup() {

  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  setMotor(false);

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED falhou");
    while (true);
  }

  display.clearDisplay();
  display.display();

  dht.begin();

  connectWiFi();
  setupWeb();

  client.setServer(mqtt_server, 1883);
}

// ================= LOOP =================
void loop() {

  server.handleClient();
  connectWiFi();

  reconnectMQTT();
  client.loop();

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t)) temperatura = t;
  if (!isnan(h)) umidade = h;

  // controle
  if (temperatura >= TEMP_ON && !motorState) setMotor(true);
  if (temperatura <= TEMP_OFF && motorState) setMotor(false);

  // MQTT (monitoramento)
  if (client.connected()) {
    client.publish((String(AIO_USERNAME)+"/feeds/temperatura").c_str(),
                   String(temperatura).c_str());

    client.publish((String(AIO_USERNAME)+"/feeds/umidade").c_str(),
                   String(umidade).c_str());
  }

  // OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0,0);
  display.print("Temp:");
  display.print(temperatura,1);

  display.setCursor(0,10);
  display.print("Hum:");
  display.print(umidade,0);

  display.setCursor(0,20);
  display.print("Motor:");
  display.print(motorState ? "ON" : "OFF");

  display.setCursor(0,30);
  display.print("Set:");
  display.print(TEMP_ON);

  display.setCursor(0,40);
  display.print(WiFi.localIP());

  display.display();

  delay(1000);
}