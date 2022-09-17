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
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 14 // GPIO 14 = D5

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

int total_devices;
DeviceAddress sensor_address;
float average_temperature =0;

#define SERIAL_DEBUG_BAUD 115200

#define DHTPIN D4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
int status = WL_IDLE_STATUS; // = 0

ThingsBoard tb(espClient);

void reconnect();

void setup()
{
  Serial.begin(SERIAL_DEBUG_BAUD);
  dht.begin();
  sensors.begin();
  total_devices = sensors.getDeviceCount();
  Serial.print(F("Locating devices..."));
  Serial.print(F("Found "));
  Serial.print(total_devices, DEC);
  Serial.println(F(" devices."));
  
  for(int i=0;i<total_devices; i++){
    if(sensors.getAddress(sensor_address, i)){
      Serial.print(F("Found device "));
      Serial.print(i, DEC);
      Serial.print(F(" with address: "));
      printAddress(sensor_address);
      Serial.println();
    } else {
      Serial.print(F("Found device at "));
      Serial.print(i, DEC);
      Serial.print(F(" but could not detect address. Check circuit connection!"));
    }
  }
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
    if (!tb.connect(THINGSBOARD_SERVER, TB_TOKEN, 1883)
    {
      Serial.println(F("Failed to connect"));
      return;
    }
  }
  sensors.requestTemperatures(); 
  for(int i=0;i<total_devices; i++){
    if(sensors.getAddress(sensor_address, i)){   
      Serial.print("Temperature for device: ");
      Serial.print(i,DEC);     
      float temperature_degreeCelsius = sensors.getTempC(sensor_address);
      Serial.print("\t");
      Serial.print(temperature_degreeCelsius);
      Serial.println(" °C");
      average_temperature += temperature_degreeCelsius;
    }
  }
  average_temperature /= total_devices;
  Serial.print(" Average Temperature is :\t");
  Serial.print(average_temperature);
  Serial.println(" °C");
  Serial.println("---------------------------");
  average_temperature = 0;
  delay(1000);  
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
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++){
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
}
