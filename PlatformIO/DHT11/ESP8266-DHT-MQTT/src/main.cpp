#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <ThingsBoard.h>
#include <ESP8266WiFi.h>
#include "creds.h"
#include "tb_creds.h"
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <string.h>

#define SERIAL_DEBUG_BAUD 115200

#define DHTPIN D5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
int status = WL_IDLE_STATUS; // = 0

ThingsBoard tb(espClient);

void reconnect();

void setup()
{
  Serial.begin(SERIAL_DEBUG_BAUD);
  delay(2000);
  Serial.println(F("DHT Test"));
  dht.begin();
  Serial.print(F("Connecting to AP : "));
  Serial.println(WiFi.SSID());
  reconnect();
}

void loop()
{
  delay(1000);
  if (!tb.connected())
  {
    // Connect to the ThingsBoard
    Serial.print(F("\nConnecting to: "));
    Serial.println(THINGSBOARD_SERVER);
    if (!tb.connect(THINGSBOARD_SERVER, TB_USER, 1883, DEV_ID, TB_PWD))
    {
      Serial.println(F("Failed to connect"));
      return;
    }
  }

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float temperatureFahrenheit = dht.readTemperature(true);
  if (isnan(humidity) || isnan(temperature) || isnan(temperatureFahrenheit))
  {
    Serial.println(F(" Failed to read from DHT"));
    return;
  }

  float heatIndexFahrenheit = dht.computeHeatIndex(temperatureFahrenheit, humidity);
  float heatIndexCelsius = dht.computeHeatIndex(temperature, humidity, false);

  char buffer[42] = {0};
  sprintf(buffer, "{\"temperature\":%.2f,\"humidity\":%.2f}", temperature, humidity);
  Serial.println(buffer);
  Serial.println();
  Serial.print(temperatureFahrenheit);
  Serial.print(F("°F  Heat index: "));
  Serial.print(heatIndexCelsius);
  Serial.print(F("°C "));
  Serial.print(heatIndexFahrenheit);
  Serial.println(F("°F"));
  Serial.println("\nSending data to server");
  tb.sendTelemetryJson(buffer);
  tb.loop();
}

void reconnect()
{
  // Loop until we're reconnected
  status = WiFi.status();
  if (status != WL_CONNECTED)
  {
    WiFi.begin(WIFI_SSID1, WIFI_PWD1);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }
    Serial.print("\nAcquired IP : ");
    Serial.println(WiFi.localIP());
  }
}
