#define TINY_GSM_MODEM_SIM800

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <ThingsBoard.h>
#include <ESP8266WiFi.h>
#include "creds.h"
#include "tb_creds.h"
#include <DHT.h>

#define DHTPIN D5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
ThingsBoard tb(espClient);
#define SERIAL_DEBUG_BAUD   115200


void setup()
{
  Serial.begin(SERIAL_DEBUG_BAUD);
  
  delay(2000);
  Serial.println(F("DHT Test"));
  dht.begin();
  
  InitWiFi();
  
}

void loop() {
  delay(1000);
  if (!tb.connected()) {
    // Connect to the ThingsBoard
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect");
      return;
    }
  }

  float rotubat = dht.readHumidity();
  float dama = dht.readTemperature();
  float damaFahrenheit = dht.readTemperature(true);
  if (isnan(rotubat) || isnan(dama) || isnan(damaFahrenheit)){
    Serial.println(" Failed to read from DHT");
    return;
  }
  
  float heatIndexFahrenheit = dht.computeHeatIndex(damaFahrenheit, rotubat);
  float heatIndexCelsius = dht.computeHeatIndex(dama,rotubat,false);

  Serial.print("{\"temp\":");
  Serial.print(dama);
  Serial.print(",\"hum\":");
  Serial.print(rotubat);
  Serial.print("}\n");
  Serial.println();
  Serial.print(F("°F  Heat index: "));
  Serial.print(heatIndexCelsius);
  Serial.print(F("°C "));
  Serial.print(heatIndexFahrenheit);
  Serial.println(F("°F"));

  Serial.println("Sending data...");

}
void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

void reconnect() {
  // Loop until we're reconnected
  status = WiFi.status();
  if ( status != WL_CONNECTED) {
    WiFi.begin(WIFI_AP, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Connected to AP");
  }
}