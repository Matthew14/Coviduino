#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266HTTPClient.h>
#include <U8x8lib.h>
#include <ArduinoJson.h>


//const char* api_url = "https://corona-stats.online/IE?source=2&format=json";
const char* api_url = "https://corona-stats.online/ireland?format=json";

const uint8_t fingerprint[20] = {0xBF, 0x56, 0x86, 0xF1, 0xA8, 0xFA, 0x3B, 0x84, 0xDB, 0x07, 0x37, 0xAF, 0x2B, 0xA9, 0x4E, 0xD5, 0x6D, 0x53, 0xDD, 0x6B};

U8X8_SSD1306_128X64_NONAME_HW_I2C oled(U8X8_PIN_NONE);

void setup() {
  Serial.begin(115200);
  //Wifi
  WiFi.begin(getssid(), getPassword());
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected");
  //Setup display
  oled.begin();
  oled.setPowerSave(0);
  oled.setFont(u8x8_font_amstrad_cpc_extended_f);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setFingerprint(fingerprint);

    HTTPClient http;
    http.begin(*client, api_url);
    Serial.print("requesting ");
    Serial.print(api_url);
    Serial.print("\n");
    int httpCode = http.GET();       
    Serial.print("http response: " );
    Serial.print(httpCode);                                                  
    if (httpCode > 0) {
    
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          DynamicJsonDocument doc(4096);
          DeserializationError error = deserializeJson(doc, http.getString());
          if (error)
          {
            Serial.println("an error occured");
            Serial.println(error.c_str());

            oled.clearDisplay();
            oled.setCursor(0,2);
            oled.print(error.c_str());
          }
          else
          {
            JsonObject root = doc.as<JsonArray>()[0];
  
            int deaths = root["deaths"];
            int cases = root["confirmed"];
  
            int yday_cases = root["confirmedByDay"][root["confirmedByDay"].size() - 2];
            int yday_deaths = root["deathsByDay"][root["deathsByDay"].size() - 2];
  
            int inc_deaths = deaths - yday_deaths;
            int inc_cases = cases - yday_cases;

            const char* country = root["country"];
  
            oled.clearDisplay();
            oled.setCursor(0, 0);
            oled.print(country);
                        
            oled.setCursor(0, 2);
            oled.print("Deaths: ");
            oled.setCursor(0, 3);
            oled.print(deaths);
            oled.print(" (+ ");
            oled.print(inc_deaths);
            oled.print(")");
            oled.setCursor(0, 6);
            oled.print("Cases: ");
            oled.setCursor(0, 7);
            oled.print(cases);
            oled.print(" (+ ");
            oled.print(inc_cases);
            oled.print(")");
  
            Serial.print("deaths: ");
            Serial.print(deaths);
  
            //update time maybe
          }
      }
    }
    http.end();
  }

 //if update changed sleep 20 hours
 //else
  delay(60 * 10 * 1000);

}
