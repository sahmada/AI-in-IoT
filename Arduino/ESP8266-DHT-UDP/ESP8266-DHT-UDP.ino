#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "DHTesp.h"

#define DHTPIN D5
#define DHTTYPE DHT11

#define WIFI_SSID "saa"
#define WIFI_PWD "**********"
DHTesp dht;

WiFiUDP udp;
bool isWiFiOK = false;

void WiFiConnect();
void WiFiEvent(WiFiEvent_t event);

void setup()
{
  Serial.begin(115200);
  Serial.println(F("DHT Test"));
  dht.setup(DHTPIN, DHTesp::DHT11);
  WiFiConnect();
}

void loop()
{
  delay(dht.getMinimumSamplingPeriod());

  float rotubat = dht.getHumidity();
  float dama = dht.getTemperature();
  //float damaFahrenheit = dht.getTemperature(true);
  if (isnan(rotubat) || isnan(dama)){
    Serial.println(" Failed to read from DHT");
    return;
  }
/*
  float heatIndexFahrenheit = dht.computeHeatIndex(damaFahrenheit, rotubat);
  float heatIndexCelsius = dht.computeHeatIndex(dama,rotubat,false);
*/
  char buffer[100] = {0};
  sprintf(buffer, "{\"temp\":%.2f ,\"hum\": %.2f}", dama, rotubat);
  if (isWiFiOK)
  {
    udp.beginPacketMulticast(IPAddress(255, 255, 255, 255), 61000, WiFi.localIP());
    udp.write(buffer);
    udp.endPacket();
  }

  Serial.println(buffer);
  delay(1000);
}

void WiFiConnect()
{
  WiFi.disconnect(true);
  WiFi.onEvent(WiFiEvent);
  WiFi.begin(WIFI_SSID, WIFI_PWD);
}

void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case WIFI_EVENT_STAMODE_GOT_IP:
    Serial.print("IP acquired :");
    Serial.println(WiFi.localIP());
    isWiFiOK = true;
    break;

  case WIFI_EVENT_STAMODE_CONNECTED:
    Serial.println("Connected");
    break;

  case WIFI_EVENT_STAMODE_DISCONNECTED:
    Serial.println("Disonnected !!!");
    isWiFiOK = false;
    WiFiConnect();
    break;
  }
}
