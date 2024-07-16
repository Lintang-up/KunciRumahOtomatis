
// If using ESP8266

#include <ESP8266WiFi.h>  // Include library Wifi for ESP8266
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#define relay D1          // Inisialisasi pin D1 untuk relay


#include <WiFiClientSecure.h>      // Library Wifi Client Secure
#include <UniversalTelegramBot.h>  // Library Universal Telegram Bot Menggunakan Versi 1.3.0
#include <ArduinoJson.h>           // Library Arduino Json Menggunakan Versi 6.21.0


/************************************************************************************************/
//#define SSID_Name "poco"        // Inizialize name of SSID
//#define PASSWORD "bisadipakai"     // Inizialize Password
#ifndef STASSID
#define STASSID "sarah"
#define STAPSK "12341234"
//#endif
#define BOTtoken "6277283663:AAHxCxp4BTpJeugADwaAMGsgQtj2jKRZQkM"           // Inizialize Token Bot Telegram from BotFather
#define CHAT_ID "1126682296"                       // Inizialize User Chat ID from IDBOT
#endif
/************************************************************************************************/

// If using ESP8266
#ifdef ESP8266
X509List cert(TELEGRAM_CERTIFICATE_ROOT);  // SSL Telegram Certificate
#endif

/************************************************************************************************/
WiFiClientSecure client;                     // Create variable name for client
UniversalTelegramBot bot(BOTtoken, client);  // Create variable name for bot

//const char* ssid = SSID_Name;     // Change SSID_Name as pointer ssid
//const char* password = PASSWORD;  // Change PASSWORD as pointer password
const char* ssid = STASSID;
const char* password = STAPSK;

int delayRequestBot = 1000;    // Delay for request with bot
unsigned long lastTimeBotRun;  // Inizialize variable lastTimeBotRun

unsigned long lightTimerExpires;
boolean lightTimerActive = false;
/************************************************************************************************/

#define hidup LOW
#define mati HIGH

void setup() {
  Serial.begin(115200);
    Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());



// If using ESP8266
#ifdef ESP8266
  // Check the SSL Certificate
  Serial.println("Menggunakan Board ESP8266");
  configTime(0, 0, "pool.ntp.org");
  client.setTrustAnchors(&cert);
#endif



  // Set relay as OUTPUT and first condition is LOW / OFF
  pinMode(relay, OUTPUT);
  digitalWrite(relay, mati);

  // Set WIFI Station mode
  WiFi.mode(WIFI_STA);
  // Connecting to WIFI
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi -> ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  // Displaying IP Address
  Serial.println("");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  bot.sendMessage(CHAT_ID, "Halo, \nAlat Sudah Siap Di Gunakan\nTekan : /start", "");
}

void loop() {
  ArduinoOTA.handle();
  // Run detect if new messages from Telegram or not
  if (millis() > lastTimeBotRun + delayRequestBot) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    // If got response from Telegram
    while (numNewMessages) {
      Serial.println("Got Renponse!");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRun = millis();
  }
}

// Berfungsi untuk mengontrol pesan dari Telegram
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  // Jika ada pesan dari telegram, tindakannya mengirim kembali pesan ke klien
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Pengguna Tidak Terdaftar!", "");  // If client from chat_id not same with chat id above
      continue;
    }

    // Create variable text to get message data from client
    String text = bot.messages[i].text;
    Serial.println(text);
    String from_name = bot.messages[i].from_name;

    if (text == F("/BukaKunciPintu")) {  // Jika klien mengirim /relayON
      digitalWrite(relay, hidup);
      bot.sendChatAction(chat_id, "typing");
      delay(500);
      bot.sendMessage(chat_id, "Kunci Pintu Terbuka");  // Pemberitahuan kondisi
    } else if (text == F("/TutupKunciPintu")) {                   // Jika klien mengirim /relayOFF
      digitalWrite(relay, mati);
      bot.sendChatAction(chat_id, "typing");
      delay(500);
      bot.sendMessage(chat_id, "Kunci Pintu Tertutup");  // Pemberitahuan kondisi
    } else if (text == F("/BukadanKunciOtomatis")) {                   // Jika klien mengirim /relayOFF
      digitalWrite(relay, hidup);
      bot.sendChatAction(chat_id, "typing");
      delay(500);
      bot.sendMessage(chat_id, "Kunci Pintu Terbuka");  // Pemberitahuan kondisi
      delay(15000);
      digitalWrite(relay, mati);
      delay(500);
      bot.sendMessage(chat_id, "Kunci Pintu Tertutup");  // Pemberitahuan kondisi
    } else if (text == F("/cekStatusPintu")) {             // Jika klien mengirim /cekStatusRelay
      bot.sendChatAction(chat_id, "typing");
      delay(500);
      if (digitalRead(relay) == hidup) {
        bot.sendMessage(chat_id, "Kunci Pintu Sudah Terbuka", "");  // Notifikasi jika kondisi Relay => LOW
      } else {
        bot.sendMessage(chat_id, "Kunci Pintu Sudah Tertutup", "");  // Notifikasi jika kondisi Relay => HIGH
      }
    } else {
      // If client send /start
      if (text == F("/start")) {
        String welcome = "Hi " + from_name + "  ,\n";
        welcome += "Silakan Tekan Ini =>\n";
        welcome += "/BukaKunciPintu\n";
        welcome += "/TutupKunciPintu\n";
        welcome += "/BukadanKunciOtomatis\n";
        welcome += "/cekStatusPintu\n";
        bot.sendMessage(chat_id, welcome);
      }
      }
    }
  }
