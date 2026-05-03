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

// ================= DHT =================
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ================= RELÉ =================
#define RELAY_PIN 5
#define RELAY_ACTIVE_LOW true

// ================= VARIÁVEIS =================
float temperatura = 0;
float umidade = 0;
bool motorState = false;

float TEMP_ON = 24.0;
float TEMP_OFF = 22.5;

// ================= TIMERS =================
unsigned long lastSensor = 0;
unsigned long lastMQTT = 0;
unsigned long lastOLED = 0;

// ================= WIFI =================
void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("\nWiFi OK");
  Serial.println(WiFi.localIP());
}

// ================= RELÉ =================
void setMotor(bool state) {
  motorState = state;

  if (RELAY_ACTIVE_LOW)
    digitalWrite(RELAY_PIN, state ? LOW : HIGH);
  else
    digitalWrite(RELAY_PIN, state ? HIGH : LOW);
}

// ================= WEB HTML =================
String pagina() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "</head><body style='font-family:Arial;text-align:center;'>";

  html += "<h2>ESP32 IoT Control</h2>";

  html += "<p>Temp: " + String(temperatura) + "</p>";
  html += "<p>Hum: " + String(umidade) + "</p>";
  html += "<p>Motor: " + String(motorState ? "ON" : "OFF") + "</p>";
  html += "<p>Setpoint: <span id='sp'>" + String(TEMP_ON) + "</span></p>";

  html += "<input type='range' min='18' max='35' value='" + String(TEMP_ON) + "' oninput='send(this.value)'>";

  html += R"rawliteral(
<script>
function send(v){
  document.getElementById('sp').innerHTML = v;
  fetch('/set?temp=' + v);
}
setInterval(()=>{location.reload();},5000);
</script>
)rawliteral";

  html += "</body></html>";
  return html;
}

// ================= WEB ROUTES =================
void handleRoot() {
  server.send(200, "text/html", pagina());
}

void handleSet() {
  if (server.hasArg("temp")) {
    float novo = server.arg("temp").toFloat();

    TEMP_ON = constrain(novo, 18, 35);
    TEMP_OFF = TEMP_ON - 1.5;

    Serial.println("SETPOINT WEB: " + String(TEMP_ON));
  }

  server.send(200, "text/plain", "OK");
}

// ================= MQTT =================
void reconnectMQTT() {
  if (client.connected()) return;

  String id = "ESP32_" + String(random(9999));

  if (client.connect(id.c_str(), AIO_USERNAME, AIO_KEY)) {
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

  dht.begin();
  connectWiFi();

  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.begin();

  client.setServer(mqtt_server, 1883);
}

// ================= LOOP (SEM DELAY) =================
void loop() {

  server.handleClient();
  connectWiFi();

  reconnectMQTT();
  client.loop();

  // ================= SENSOR =================
  if (millis() - lastSensor > 2000) {
    lastSensor = millis();

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t)) temperatura = t;
    if (!isnan(h)) umidade = h;

    if (temperatura >= TEMP_ON && !motorState) setMotor(true);
    if (temperatura <= TEMP_OFF && motorState) setMotor(false);
  }

  // ================= MQTT =================
  if (millis() - lastMQTT > 5000) {
    lastMQTT = millis();

    if (client.connected()) {
      client.publish((String(AIO_USERNAME)+"/feeds/temperatura").c_str(),
                     String(temperatura).c_str());

      client.publish((String(AIO_USERNAME)+"/feeds/umidade").c_str(),
                     String(umidade).c_str());
    }
  }

  // ================= OLED =================
  if (millis() - lastOLED > 1000) {
    lastOLED = millis();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    display.setCursor(0,0);
    display.print("Temp: "); display.print(temperatura);

    display.setCursor(0,10);
    display.print("Hum: "); display.print(umidade);

    display.setCursor(0,20);
    display.print("Motor: "); display.print(motorState ? "ON" : "OFF");

    display.setCursor(0,30);
    display.print("Set: "); display.print(TEMP_ON);

    display.setCursor(0,40);
    display.print(WiFi.localIP());

    display.display();
  }
}