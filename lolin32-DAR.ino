/*
void setup(){
  Serial.begin(115200);
  
  
  }

void loop(){
  Serial.println("hello...");
  delay(1000);
  }
*/
  

#include <LittleFS.h>

#include <FS.h>                       //library to access the filesystem
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include "SPIFFS.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>       //https://github.com/alanswx/ESPAsyncWiFiManager
#include <ESPAsyncWiFiManager.h>     //https://github.com/alanswx/ESPAsyncWiFiManager
#include <ArduinoJson.h>             //https://github.com/bblanchon/ArduinoJson
#include <UniversalTelegramBot.h>    //https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <SimpleTimer.h>             //https://github.com/jfturcot/SimpleTimer

//define custom fields
char chat_id[100];
char bot_token[100];
const char root_ca[] = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIEADCCAuigAwIBAgIBADANBgkqhkiG9w0BAQUFADBjMQswCQYDVQQGEwJVUzEh\n"\
"MB8GA1UEChMYVGhlIEdvIERhZGR5IEdyb3VwLCBJbmMuMTEwLwYDVQQLEyhHbyBE\n"\
"YWRkeSBDbGFzcyAyIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MB4XDTA0MDYyOTE3\n"\
"MDYyMFoXDTM0MDYyOTE3MDYyMFowYzELMAkGA1UEBhMCVVMxITAfBgNVBAoTGFRo\n"\
"ZSBHbyBEYWRkeSBHcm91cCwgSW5jLjExMC8GA1UECxMoR28gRGFkZHkgQ2xhc3Mg\n"\
"MiBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTCCASAwDQYJKoZIhvcNAQEBBQADggEN\n"\
"ADCCAQgCggEBAN6d1+pXGEmhW+vXX0iG6r7d/+TvZxz0ZWizV3GgXne77ZtJ6XCA\n"\
"PVYYYwhv2vLM0D9/AlQiVBDYsoHUwHU9S3/Hd8M+eKsaA7Ugay9qK7HFiH7Eux6w\n"\
"wdhFJ2+qN1j3hybX2C32qRe3H3I2TqYXP2WYktsqbl2i/ojgC95/5Y0V4evLOtXi\n"\
"EqITLdiOr18SPaAIBQi2XKVlOARFmR6jYGB0xUGlcmIbYsUfb18aQr4CUWWoriMY\n"\
"avx4A6lNf4DD+qta/KFApMoZFv6yyO9ecw3ud72a9nmYvLEHZ6IVDd2gWMZEewo+\n"\
"YihfukEHU1jPEX44dMX4/7VpkI+EdOqXG68CAQOjgcAwgb0wHQYDVR0OBBYEFNLE\n"\
"sNKR1EwRcbNhyz2h/t2oatTjMIGNBgNVHSMEgYUwgYKAFNLEsNKR1EwRcbNhyz2h\n"\
"/t2oatTjoWekZTBjMQswCQYDVQQGEwJVUzEhMB8GA1UEChMYVGhlIEdvIERhZGR5\n"\
"IEdyb3VwLCBJbmMuMTEwLwYDVQQLEyhHbyBEYWRkeSBDbGFzcyAyIENlcnRpZmlj\n"\
"YXRpb24gQXV0aG9yaXR5ggEAMAwGA1UdEwQFMAMBAf8wDQYJKoZIhvcNAQEFBQAD\n"\
"ggEBADJL87LKPpH8EsahB4yOd6AzBhRckB4Y9wimPQoZ+YeAEW5p5JYXMP80kWNy\n"\
"OO7MHAGjHZQopDH2esRU1/blMVgDoszOYtuURXO1v0XJJLXVggKtI3lpjbi2Tc7P\n"\
"TMozI+gciKqdi0FuFskg5YmezTvacPd+mSYgFFQlq25zheabIZ0KbIIOqPjCDPoQ\n"\
"HmyW74cNxA9hi63ugyuV+I6ShHI56yDqg+2DzZduCLzrTia2cyvk0/ZM/iZx4mER\n"\
"dEr/VxqHD3VILs9RaRegAhJhldXRQLIQTO7ErBBDpqWeCtWVYpoNz4iCxTIM5Cuf\n"\
"ReYNnyicsbkqWletNw+vHX/bvZ8=\n"\
"-----END CERTIFICATE-----\n";
//String chat_id;
//String bot_token;

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

AsyncWebServer server(80);
DNSServer dns;
SimpleTimer timer;

void setup() {
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1); //Wake up if movement detected
  Serial.begin(115200);
  Serial.println("\n Starting");
  //clean FS, for testing
  //SPIFFS.format();
  //read configuration from FS json
  Serial.println("mounting FS...");
    if (SPIFFS.begin()) {
      Serial.println("mounted file system");
      if (SPIFFS.exists("/config.json")) {
        //file exists, reading and loading
        Serial.println("reading config file");
        File configFile = SPIFFS.open("/config.json", "r");
          if (configFile) {
            Serial.println("opened config file");
            size_t size = configFile.size();
            // Allocate a buffer to store contents of the file.
            std::unique_ptr<char[]> buf(new char[size]);
            configFile.readBytes(buf.get(), size);
            DynamicJsonBuffer jsonBuffer;
            JsonObject& json = jsonBuffer.parseObject(buf.get());
            json.printTo(Serial);
        
        
        
            if (json.success()) {
              Serial.println("\nparsed json");
              strcpy(chat_id, json["chat_id"]);
              strcpy(bot_token, json["bot_token"]);
    
              Serial.print("loaded_chat_id=");
              Serial.println(chat_id);
              Serial.print("loaded_token_id=");
              Serial.println(chat_id);
              shouldSaveConfig = false;
              
            } else {
            Serial.println("failed to load json config");
          }
        }
      } else {Serial.println("failed to load json");shouldSaveConfig=true;}
    } else {
      Serial.println("failed to mount FS");
  }
  //end read

  //Telegram parameters
  AsyncWiFiManagerParameter custom_chat_id("chat_id", "chat id", chat_id, 100);
  AsyncWiFiManagerParameter custom_bot_token("bot_token", "bot token", bot_token, 100);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  AsyncWiFiManager wifiManager(&server, &dns);

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&custom_chat_id);
  wifiManager.addParameter(&custom_bot_token);

  //scheduled reset
  int horas = 1; // hours for scheduled reset
  timer.setInterval(horas * 3600000, reinicio);

  //WiFiManager

  AsyncWiFiManagerParameter custom_text("<p>Seleccione la red WiFi para conectarse.</p>");
  wifiManager.addParameter(&custom_text);
  //reset saved settings
  //wifiManager.resetSettings();
  //set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("Detector AntiRobos AP");
  //or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  //read updated parameters

  strcpy(chat_id, custom_chat_id.getValue());
  strcpy(bot_token, custom_bot_token.getValue());
  Serial.print("custom_chat_id=");
  Serial.println(custom_chat_id.getValue());
  Serial.print("custom_chat_id=");
  Serial.println(custom_bot_token.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["chat_id"] = chat_id;
    json["bot_token"] = bot_token;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    Serial.println("config file for writing success");
    //end save
  }
  Serial.println("local ip");
  Serial.println(WiFi.localIP());

}


void loop() {
  timer.run();

  // Initialize Telegram BOT
  //WiFiClientSecure client;
  WiFiClient client;
  //client.setCACert(root_ca);
  //client.setInsecure();
  //UniversalTelegramBot bot(bot_token, client);
  int Bot_mtbs = 1000; //mean time between scan messages
  long Bot_lasttime;   //last time messages' scan has been done

  //Send alert
  //String message = "Movimiento detectado";

  if (!client.connect("192.168.1.224", 8000)) {
    Serial.println("connection failed");
    return;
  }


  String data = "message=hola%20desde%20lolin32-1&token=token1";
   Serial.print("Requesting POST: ");
   // Send request to the server:
   client.println("POST /api/alert HTTP/1.1");
   client.println("Host: localhost");
   client.println("Accept: */*");
   client.println("Content-Type: application/x-www-form-urlencoded");
   client.print("Content-Length: ");
   client.println(data.length());
   client.println();
   client.print(data);

    delay(500); // Can be changed
    if (client.connected()) { 
      client.stop();  // DISCONNECT FROM THE SERVER
    }
  Serial.println();
  Serial.println("closing connection");
  /*
  if (bot.sendMessage(chat_id, message, "Markdown")) {
    Serial.println("TELEGRAM Successfully sent");
  } else {
    Serial.println("Fallo TELEGRAM");
    
    Serial.print("Bot_token=");
    Serial.println(bot_token);
    
    Serial.print("chat_id=");
    Serial.println(chat_id);
    reinicio();
  }
  */
  Serial.println("Activando modo ahorro... en 15 segundos");
  delay(15000);
  esp_deep_sleep_start();
  Serial.println("Este mensaje nunca saldra");
}

void reinicio() {
  ESP.restart();
}
