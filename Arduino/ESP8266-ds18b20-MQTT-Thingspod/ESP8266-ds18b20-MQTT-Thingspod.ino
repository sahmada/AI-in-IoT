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

// Baud rate
#define SERIAL_DEBUG_BAUD 115200

// DHT
#define DHTPIN D4  // Must not be same DS18B20 pin
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// DS18B20 thermal sensor
#define ONE_WIRE_BUS 14  // GPIO14 = D5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int total_devices;
float average_temperature =0;
DeviceAddress sensor_address;

// WiFi
WiFiClient espClient;
int status = WL_IDLE_STATUS; // = 0

// ThingsBoard
ThingsBoard tb(espClient);

//void reconnect();


void setup()
{
  Serial.begin(SERIAL_DEBUG_BAUD);
  sensors.begin();
  Serial.print(F("Connecting to AP : "));
  Serial.println(WiFi.SSID());
  reconnect();
  Serial.println(F("DHT Test"));
  dht.begin();
  
  total_devices = sensors.getDeviceCount();
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(total_devices, DEC);
  Serial.println(" devices.");

  for(int i=0;i<total_devices; i++){
    if(sensors.getAddress(sensor_address, i)){
      Serial.print("Found device ");
      Serial.print(i, DEC);
      Serial.print(" with address: ");
      printAddress(sensor_address);
      Serial.println();
    } else {
      Serial.print("Found device at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address. Check circuit connection!");
    }
  }
}

void loop(){ 
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


void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++){
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
