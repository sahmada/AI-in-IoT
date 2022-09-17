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
// define baud rate
#define SERIAL_DEBUG_BAUD 115200
// define dht22 sensor
#define DHTPIN22 D3
#define DHTTYPE22 DHT22
DHT dht22(DHTPIN22, DHTTYPE22);
// define dht11 sensor
#define DHTPIN11 D2
#define DHTTYPE11 DHT11
DHT dht11(DHTPIN11, DHTTYPE11);
// define HOT temperature
float hotTemperature = 30.0;

WiFiClient espClient;
int status = WL_IDLE_STATUS; // = 0

ThingsBoard tb(espClient);

void reconnect();
void blinking();

void setup()
{
  Serial.begin(SERIAL_DEBUG_BAUD);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(2000);
  Serial.println(F("DHT Test"));
  dht11.begin();
  dht22.begin();
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
  // Reading data from DHT11
  float humidity11 = dht11.readHumidity();
  float temperature11 = dht11.readTemperature();
  float temperatureFahrenheit11 = dht11.readTemperature(true);
  if (isnan(humidity11) || isnan(temperature11) || isnan(temperatureFahrenheit11))
  {
    Serial.println(F(" Failed to read from DHT11"));
    return;
  }
  float heatIndexFahrenheit11 = dht11.computeHeatIndex(temperatureFahrenheit11, humidity11);
  float heatIndexCelsius11 = dht11.computeHeatIndex(temperature11, humidity11, false);
  // Reading data from DHT22
  float humidity22 = dht22.readHumidity();
  float temperature22 = dht22.readTemperature();
  float temperatureFahrenheit22 = dht22.readTemperature(true);
  if (isnan(humidity22) || isnan(temperature22) || isnan(temperatureFahrenheit22))
  {
    Serial.println(F(" Failed to read from DHT22"));
    return;
  }
  float heatIndexFahrenheit22 = dht22.computeHeatIndex(temperatureFahrenheit22, humidity22);
  float heatIndexCelsius22 = dht22.computeHeatIndex(temperature22, humidity22, false);
  // Calculating the mean/average values
  float temperature = (temperature11 + temperature22) / 2;
  float humidity = (humidity11 + humidity22) / 2;
  float temperatureFahrenheit = (temperatureFahrenheit11 + temperatureFahrenheit22) / 2;
  float heatIndexCelsius = (heatIndexCelsius11 + heatIndexCelsius22) / 2;
  float heatIndexFahrenheit = (heatIndexFahrenheit11 + heatIndexFahrenheit22) / 2;
  // blinking if temperature is high
  if (temperature >= hotTemperature)
  {
    blinking();
  }
  // making buffer string ready to be sent
  int bufferLength = 42;
  char buffer[bufferLength] = {0};
  sprintf(buffer, "{\"temperature11\":%.2f,\"humidity11\":%.2f}", temperature11, humidity11);
  Serial.println(buffer);
  Serial.println();
  tb.sendTelemetryJson(buffer);
  Serial.println(buffer);
  memset(buffer, 0, bufferLength);
  sprintf(buffer, "{\"temperature22\":%.2f,\"humidity22\":%.2f}", temperature22, humidity22);
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
  memset(buffer, 0, bufferLength);
}

void blinking()
{
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
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
