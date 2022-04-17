#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "creds.h"
#include "tb_creds.h"

// ESP8266 NODEMCU
#define GPIO1 1 // TXD0
#define GPIO2 2 // D4, LED_BUILTIN
#define GPIO3 3 // RXD0
// ESP8266 NODEMCU
#define GPIO1_PIN 1
#define GPIO2_PIN 2
#define GPIO3_PIN 3

WiFiClient wifiClient;

PubSubClient client(wifiClient);

int status = WL_IDLE_STATUS;

// We assume that all GPIOs are LOW
boolean gpioState[] = {false, false, false};

String get_gpio_status()
{
  StaticJsonDocument<200> jsonDoc;
  jsonDoc[String(GPIO1_PIN)] = gpioState[1] ? true : false; // NodeMCU is backwards, default should be HIGH : LOW for arduino
  jsonDoc[String(GPIO2_PIN)] = gpioState[2] ? true : false; // NodeMCU is backwards, default should be HIGH : LOW for arduino
  jsonDoc[String(GPIO3_PIN)] = gpioState[3] ? true : false; // NodeMCU is backwards, default should be HIGH : LOW for arduino
  char payload[256];
  serializeJson(jsonDoc, payload);
  String strPayload = String(payload);
  Serial.print("Get gpio status: ");
  Serial.println(strPayload);
  return strPayload;
}

void set_gpio_status(int pin, boolean enabled)
{
  if (pin == GPIO1_PIN)
  {
    // Output GPIOs state
    digitalWrite(GPIO1, enabled ? LOW : HIGH); // NodeMCU is backwards, Arduino is HIGH : LOW
    // Update GPIOs state
    gpioState[1] = enabled;
  }

  if (pin == GPIO2_PIN)
  {
    // Output GPIOs state
    digitalWrite(GPIO2, enabled ? LOW : HIGH); // NodeMCU is backwards, Arduino is HIGH : LOW
    // Update GPIOs state
    gpioState[2] = enabled;
  }

  if (pin == GPIO3_PIN)
  {
    // Output GPIOs state
    digitalWrite(GPIO3, enabled ? LOW : HIGH); // NodeMCU is backwards, Arduino is HIGH : LOW
    // Update GPIOs state
    gpioState[3] = enabled;
  }
}
// The callback for when a PUBLISH message is received from the server.
void on_message(const char *topic, byte *payload, unsigned int length)
{
  Serial.println("On message");

  char json[length + 1];
  strncpy(json, (char *)payload, length);
  json[length] = '\0';

  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(json);

  // Decode JSON request
  StaticJsonDocument<200> jsonDoc;
  auto error = deserializeJson(jsonDoc, (char *)json);

  if (error)
  {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return;
  }

  // Check request method
  String methodName = String((const char *)jsonDoc["method"]);

  // Reply with GPIO status
  if (methodName.equals("getGpioStatus"))
  {
    String responseTopic = String(topic);
    responseTopic.replace("request", "response");
    client.publish(responseTopic.c_str(), get_gpio_status().c_str());
  }
  else if (methodName.equals("setGpioStatus"))
  {
    // Update GPIO status and reply
    set_gpio_status(jsonDoc["params"]["pin"], jsonDoc["params"]["enabled"]);
    String responseTopic = String(topic);
    responseTopic.replace("request", "response");
    client.publish(responseTopic.c_str(), get_gpio_status().c_str());
    client.publish("v1/devices/me/attributes", get_gpio_status().c_str());
  }
}

void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    status = WiFi.status();
    if (status != WL_CONNECTED)
    {
      WiFi.begin(WIFI_AP, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED)
      {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    Serial.print("Connecting to ThingsBoard node ...");
    // Attempt to connect (clientId, username, password)
    if (client.connect("ESP8266 Device", TOKEN, NULL))
    {
      Serial.println("[DONE]");
      // Subscribing to receive RPC requests
      client.subscribe("v1/devices/me/rpc/request/+");
      // Sending current GPIO status
      Serial.println("Sending current GPIO status ...");
      client.publish("v1/devices/me/attributes", get_gpio_status().c_str());
    }
    else
    {
      Serial.print("[FAILED] [ rc = ");
      Serial.print(client.state());
      Serial.println(" : retrying in 5 seconds]");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.print("Setup");

  // Set output mode for all GPIO pins
  pinMode(GPIO1, OUTPUT);
  digitalWrite(GPIO1, HIGH); // NodeMCU is backwards, Arduino is LOW
  pinMode(GPIO2, OUTPUT);
  digitalWrite(GPIO2, HIGH); // NodeMCU is backwards, Arduino is LOW
  pinMode(GPIO3, OUTPUT);
  digitalWrite(GPIO3, HIGH); // NodeMCU is backwards, Arduino is LOW

  delay(10);

  InitWiFi();
  client.setServer(THINGSBOARD_SERVER, 1883);
  client.setCallback(on_message);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }

  client.loop();
}
