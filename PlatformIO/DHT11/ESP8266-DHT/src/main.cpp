#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <SPI.h>
#include <Wire.h>
#include <DHT.h>

#define DHTPIN D5
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);


void setup() {
  Serial.begin(115200);
  Serial.println(F("DHT Test"));

  dht.begin();
}

void loop() {
  delay(2000);

  float rotubat = dht.readHumidity();
  float dama = dht.readTemperature();
  float damaFahrenheit = dht.readTemperature(true);
  if (isnan(rotubat) || isnan(dama) || isnan(damaFahrenheit)){
    Serial.println(" Failed to read from DHT");
    return;
  }
/*
  float heatIndexFahrenheit = dht.computeHeatIndex(damaFahrenheit, rotubat);
  float heatIndexCelsius = dht.computeHeatIndex(dama,rotubat,false);

  Serial.print(F("Humidity: "));
  Serial.print(rotubat);
  Serial.print(F("% Temperature: "));
  Serial.print(dama);
  Serial.print(F("°C "));
  Serial.print(damaFahrenheit);
  Serial.print(F("°F  Heat index: "));
  Serial.print(heatIndexCelsius);
  Serial.print(F("°C "));
  Serial.print(heatIndexFahrenheit);
  Serial.println(F("°F"));
*/

  Serial.print("{\"temp\":");
  Serial.print(dama);
  Serial.print(",\"hum\":");
  Serial.print(rotubat);
  Serial.print("}\n");

}