// #define TINY_GSM_MODEM_SIM800
#define TINY_GSM_MODEM_MC60

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_MAX31865.h>
#include <ThingsBoard.h>
#include <TinyGsmClient.h>
#include <ESP8266WiFi.h>
#include "creds.h"
#include "tb_creds.h"

#define SerialMon Serial
#include <SoftwareSerial.h>
SoftwareSerial SerialAT(4, 2); // RX, TX
#define GSM_PIN ""
const char apn[] = "mtnirancell";
const char gprsUser[] = "";
const char gprsPass[] = "";

#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
// TinyGsm modem(SerialAT);

TinyGsmClient client(modem);

// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31865 thermo = Adafruit_MAX31865(5, 13, 12, 14);
// use hardware SPI, just pass in the CS pin
//Adafruit_MAX31865 thermo = Adafruit_MAX31865(9);

// The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
#define RREF 430.0
// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
#define RNOMINAL 100.0

// Initialize ThingsBoard instance
ThingsBoard tb(client);

bool isGSM_OK = false;

void connectToGSM();

void hw_wdt_disable()
{
  *((volatile uint32_t *)0x60000900) &= ~(1); // Hardware WDT OFF
}

void hw_wdt_enable()
{
  *((volatile uint32_t *)0x60000900) |= 1; // Hardware WDT ON
}

void setup()
{
  WiFi.mode(WIFI_OFF);
  ESP.wdtDisable();
  hw_wdt_disable();

  SerialMon.begin(115200);
  delay(10);

  SerialAT.begin(115200);
  modem.setBaud(115200);
  delay(6000);

  connectToGSM();

  thermo.begin(MAX31865_3WIRE); // set to 2WIRE or 4WIRE as necessary
}

void loop()
{
  // Make sure we're still registered on the network
  if (!modem.isNetworkConnected() || !modem.isGprsConnected())
  {
    SerialMon.println("disconnected");
    isGSM_OK = false;
    connectToGSM();
    return;
  }

  if (isGSM_OK)
  {
    float temp = thermo.temperature(RNOMINAL, RREF);
    uint8_t fault = thermo.readFault();

    if (fault)
    {
      thermo.clearFault();
    }

    char buffer[30] = {0};

    sprintf(buffer, "{\"temp\":%.2f,\"fault\":%d}", temp, fault);

    SerialMon.println(buffer);
    if (!tb.connected())
    {
      // if (!tb.connect("tb-srv.abreman.ir", TB_USER, 1883, "", TB_PWD))
      if (!tb.connect("185.8.175.221", TB_USER, 1883, "", TB_PWD))
      {
        SerialMon.println("Failed to connect to thingsboard");
        return;
      }
    }

    tb.sendTelemetryJson(buffer);
    tb.loop();
  }

  delay(1000);
}

void connectToGSM()
{
  SerialMon.println("Initializing modem...");
  modem.restart();

  SerialMon.println("Waiting for network...");
  if (!modem.waitForNetwork())
  {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isNetworkConnected())
  {
    SerialMon.println("Network connected");
  }

  // GPRS connection parameters are usually set after network registration
  SerialMon.print(F("Connecting to "));
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass))
  {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isGprsConnected())
  {
    SerialMon.println("GPRS connected");
  }

  isGSM_OK = true;
}