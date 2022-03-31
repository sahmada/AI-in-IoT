/*************************************************** 
  This is a library for the Adafruit PT100/P1000 RTD Sensor w/MAX31865

  Designed specifically to work with the Adafruit RTD Sensor
  ----> https://www.adafruit.com/products/3328

  This sensor uses SPI to communicate, 4 pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_MAX31865.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "creds.h"

// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31865 thermo = Adafruit_MAX31865(5, 13, 12, 14);
// use hardware SPI, just pass in the CS pin
//Adafruit_MAX31865 thermo = Adafruit_MAX31865(9);

// The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
#define RREF 430.0
// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
#define RNOMINAL 100.0

WiFiUDP Udp;
bool isWiFiOK = false;

void WiFiConnect();
void WiFiEvent(WiFiEvent_t event);

void setup()
{
  Serial.begin(115200);

  WiFiConnect();

  thermo.begin(MAX31865_3WIRE); // set to 2WIRE or 4WIRE as necessary
}

void loop()
{
  float temp = thermo.temperature(RNOMINAL, RREF);
  uint8_t fault = thermo.readFault();

  if (fault)
  {
    thermo.clearFault();
  }

  char buffer[30] = {0};

  sprintf(buffer, "{\"temp\":%.2f,\"fault\":%d}", temp, fault);

  Serial.println(buffer);

  // Serial.print("{\"temp\":");
  // Serial.print(temp);
  // Serial.print(",\"fault\":");
  // Serial.print(fault);
  // Serial.print("}\n");

  if (isWiFiOK)
  {
    Udp.beginPacketMulticast(IPAddress(255, 255, 255, 255), 61000, WiFi.localIP());
    Udp.write(buffer);
    Udp.endPacket();
  }

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
  case WIFI_EVENT_STAMODE_CONNECTED:
    Serial.println("connected");
    break;

  case WIFI_EVENT_STAMODE_GOT_IP:
    Serial.print("got ip: ");
    Serial.println(WiFi.localIP());
    isWiFiOK = true;
    break;

  case WIFI_EVENT_STAMODE_DISCONNECTED:
    Serial.println("disconnected");
    isWiFiOK = false;
    WiFiConnect();
    break;
  }
}