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

// ===== TELEGRAM =====
if (WiFi.status() == WL_CONNECTED &&
    millis() - lastTelegram > telegramInterval) {

  lastTelegram = millis();

  int numNewMessages =
      bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages) {

    for (int i = 0; i < numNewMessages; i++) {

      String text = bot.messages[i].text;
      String chat_id = bot.messages[i].chat_id;

      if (chat_id != CHAT_ID)
        continue;

      // ===== STATUS =====
      if (text == "/status") {

        String msg = "";

        msg += "Temp: ";
        msg += String(temperatura);
        msg += " C\n";

        msg += "Hum: ";
        msg += String(umidade);
        msg += " %\n";

        msg += "Set: ";
        msg += String(TEMP_ON);
        msg += " C\n";

        msg += "Motor: ";
        msg += (motorState ? "ON" : "OFF");

        bot.sendMessage(chat_id, msg, "");
      }
    }

    numNewMessages =
      bot.getUpdates(bot.last_message_received + 1);
  }
}