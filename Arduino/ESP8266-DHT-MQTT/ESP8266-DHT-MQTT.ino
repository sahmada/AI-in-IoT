#include <ArduinoJson.h>
#include <ThingsBoard.h>
#include <ESP8266WiFi.h>
#include "creds.h"
#include "tb_creds.h"
#include <DHT.h>


#define SERIAL_DEBUG_BAUD 115200

#define DHTPIN D4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
int status = WL_IDLE_STATUS; // = 0
unsigned long lastSend;


ThingsBoard tb(espClient);
String payload;

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

  delay(1000);
  if (!tb.connected())
  {
    // Connect to the ThingsBoard
    Serial.print(F("\nConnecting to: "));
    Serial.println(THINGSBOARD_SERVER);
    //if (!tb.connect(THINGSBOARD_SERVER, TOKEN))
    if (!tb.connect(THINGSBOARD_SERVER, TB_USER, 1883, DEV_ID, TB_PWD))
    {
      Serial.println(F("Failed to connect"));
      return;
    }else{
      Serial.println(F("Successfully connected to TB Server\n"));



      
    }
  }
}

void loop()
{
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float temperatureFahrenheit = dht.readTemperature(true);
  if (isnan(humidity) || isnan(temperature) || isnan(temperatureFahrenheit))
  {
    Serial.println(F(" Failed to read from DHT"));
    return;
  }
/*
  float heatIndexFahrenheit = dht.computeHeatIndex(temperatureFahrenheit, humidity);
  float heatIndexCelsius = dht.computeHeatIndex(temperature, humidity, false);

  Serial.print(temperatureFahrenheit);
  Serial.print(F("°F  Heat index: "));
  Serial.print(heatIndexCelsius);
  Serial.print(F("°C "));
  Serial.print(heatIndexFahrenheit);
  Serial.println(F("°F"));
  Serial.println("\nSending data to server");
*/
  char buffer[64]={0};
  StaticJsonDocument<64> doc;
  doc["temperature"]= temperature; //24.12345678
  doc["humidity"]=humidity;        //33.12345678
  serializeJson(doc, buffer);
  tb.sendTelemetryJson(buffer);
  tb.loop();
  serializeJson(doc, Serial);
  Serial.println();
 
  delay(1000);
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
