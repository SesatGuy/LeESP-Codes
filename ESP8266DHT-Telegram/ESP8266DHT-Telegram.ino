// ESP8266 DHT Telegram v1
// Created By SesatGuy In Github.com
// 

#ifdef ESP32
  #include <WiFi.h>
  #include <HTTPClient.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>
#include "DHT.h"

// Replace with your network credentials
const char* ssid = "Your-Wifi-Name";
const char* password = "Your-Wifi-Password";

// Initialize Telegram BOT
#define BOTtoken "Bot-Token"

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "You-ChatID"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

#define DHTTYPE DHT11   // DHT11
#define DHTTYPE DHT22   // DHT22
#define DHTPIN 2  // D5 For Data
DHT dht(DHTPIN, DHTTYPE);

// Show RAM Usage
#ifdef ESP32
  int max_memory = 512000; // ESP32 max ram, - value is in bytes
#else
  int max_memory = 80000; // ESP8266 max ram, - value is in bytes
#endif

void setupWifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  pinMode(BUILTIN_LED, OUTPUT);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(BUILTIN_LED, LOW);
}

void setup() {
  Serial.begin(9600);
  dht.begin();
  setupWifi();

  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
    client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  #endif

  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif
}


// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands to control your room.\n\n";
      welcome += "/roomtemp for check room temp \n";
      welcome += "/stats \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/roomtemp") {
      float t = dht.readTemperature();
      float h = dht.readHumidity();
      float hic = dht.computeHeatIndex(t, h, false);
      
      String room = "Your Room Statistics.\n\n";
      room += "Temperature: ";
      room += float(t);
      room += "ºC \nHumidity: ";
      room += float(h);
      room += "% \nHeat Index: ";
      room += float(hic);
      room += "ºC \n";
      bot.sendMessage(chat_id, room, "");
    }
    
    if (text == "/abtbot") {
     // Show Ram Usage
     // Testing Things
      int used_memory = max_memory - ESP.getFreeHeap();
      float memory_max_kb_float = (float)max_memory / 1000;
      float memory_usage_kb_float = (float)used_memory / 1000;
      float memory_usage_float = (float)used_memory / max_memory * 100;
      int memory_usage_int = memory_usage_float;
      int memory_usage_kb_int = memory_usage_kb_float;

      String memory_usage = "RAM: " + String(memory_usage_int);
      memory_usage.concat("% (");
      memory_usage.concat(String(memory_usage_kb_int));
      memory_usage.concat("KB/");
      memory_usage.concat(String(memory_max_kb_float, 0));
      memory_usage.concat("KB)");

      // Send Message
      String stats = " About ESP_Helperbot Stats \n\n";
      stats += memory_usage;
      
      bot.sendMessage(chat_id, stats, "");
     }     
  }
}

void loop() {
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
