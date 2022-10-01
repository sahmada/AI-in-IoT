#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <ThingsBoard.h>
#include <ESP8266WiFi.h>
#include "creds.h"
#include "tb_creds.h"
#include <string.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define SERIAL_DEBUG_BAUD 115200

#define ONE_WIRE_BUS 14 // GPIO 14 = D5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
int total_devices;
char devID[3][16];
DeviceAddress sensor_address;

WiFiClient espClient;
int status = WL_IDLE_STATUS; // = 0

ThingsBoard tb(espClient);


void printAddress(DeviceAddress sensor_address)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (sensor_address[i] < 16)
      Serial.print("0");
    //Serial.print(sensor_address[i], HEX);
    devID[i][16] = sensor_address[i];

  }
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

void setup()
{
  Serial.begin(SERIAL_DEBUG_BAUD);
  sensors.begin();
  total_devices = sensors.getDeviceCount();

  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(total_devices, DEC);
  Serial.println(" devices.");

  for (int i = 0; i < total_devices; i++)
  {
    if (sensors.getAddress(sensor_address, i))
    {
      Serial.print("Found device ");
      Serial.print(i, DEC);
      Serial.print(" with address: ");
      printAddress(sensor_address);

      Serial.println();
    }
    else
    {
      Serial.print("Found device at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address. Check circuit connection!");
    }
  }

  Serial.println(devID[1][16]);
  Serial.println(devID[2][16]);
  Serial.println(devID[3][16]);
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

  Serial.print(F("Connecting to AP : "));
  Serial.println(WiFi.SSID());
  reconnect();
}

void loop()
{
  sensors.requestTemperatures();
  char buffer[44];
  sprintf(buffer, "{");
  for (int i = 0; i < total_devices; i++)
  {
    if (sensors.getAddress(sensor_address, i))
    {
      Serial.print("Temperature for device: ");
      Serial.print(i, DEC);
      float temperature_degreeCelsius = sensors.getTempCByIndex(i);
      // float temperature_degreeCelsius = sensors.getTempC(sensor_address);
      Serial.print("\t");
      Serial.print(temperature_degreeCelsius);
      Serial.println(" °C");
      sprintf(buffer + strlen(buffer), "\"temp%d", i);
      sprintf(buffer + strlen(buffer), "\":%.2f,", temperature_degreeCelsius);
    }
    
    
    //buffer[strlen(buffer) - 1] = '}';
    //buffer[strlen(buffer)]='\0';
    //sprintf(buffer + strlen(buffer), "}");
    
    
  }
  buffer[strlen(buffer)-1] = '\0';
  sprintf(buffer + strlen(buffer),"}");
  Serial.println(strlen(buffer));
  delay(1000);
  Serial.println(buffer);
  
  tb.sendTelemetryJson(buffer);
  tb.loop();
}

